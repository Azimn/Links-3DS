# Links 2.30 porting plan

## Scope

The first browser milestone targets original 3DS and original 2DS hardware. It uses a 400 by 240 top-screen browser viewport and a CPU-owned RGB565 backing surface. The bottom screen is reserved for browser controls in a later milestone.

## Non-goals for Milestone 1

Milestone 1 does not include TLS, JavaScript, image decoders, FreeType, Citro2D, multiple windows, downloads, bookmarks, or a bottom-screen keyboard. Those features remain disabled until the native graphics device is stable.

## Upstream integration

The port is maintained as patches against the pinned Links 2.30 release.

The intended patch series is:

1. `0001-configure-add-grdrv-3ds.patch`
2. `0002-drivers-register-grdrv-3ds.patch`
3. `0003-add-graphics-3ds-driver.patch`
4. `0004-add-libctru-input-pump.patch`
5. `0005-add-devkitarm-build-wrapper.patch`

The new backend must be registered in Links' compile-time graphics driver table. It must not depend on a constructor-based registration function.

## Graphics design

Links renders into ordinary CPU bitmaps. The 3DS driver copies clipped bitmap regions and primitive drawing operations into one RGB565 software surface. Presentation converts dirty regions into the rotated BGR8 framebuffer returned by libctru.

The initial implementation favors correctness over speed. Once Links renders reliably, the presentation path can move to a tiled GPU texture or optimized transfer without changing the Links graphics-driver boundary.

The driver owns:

- a 400 by 240 RGB565 surface
- the current clip rectangle
- dirty-region tracking
- pointer position and button state
- one active graphics device during the first milestone

## Input design

Input must be delivered through the handlers installed on `struct graphics_device` by Links. The 3DS backend polls HID from a callback scheduled through Links' existing event scheduler.

Initial mapping:

- Circle Pad: pointer movement
- A: left mouse press and release
- B: browser back key
- D-pad: keyboard navigation or scrolling
- L and R: page movement
- X: URL-entry command when UI support exists
- START: close request

The port must not replace Links' timer API or wrap `links_main()` in another permanent loop.

## Milestone acceptance criteria

### Milestone 0

- devkitARM builds a `.3dsx`
- a correctly oriented test pattern fills the top screen
- Circle Pad movement is visible
- A changes state
- START exits cleanly

### Milestone 1

- Links selects `3ds` as a graphical driver
- one graphics device opens at 400 by 240
- clipping, fill, horizontal line, vertical line, bitmap drawing, and scrolling render correctly
- the Links menu or blank browser window appears
- A produces a complete mouse click
- START requests clean shutdown
- repeated redraws do not leak memory

### Milestone 2

- HTTP networking through libctru sockets
- DNS and nonblocking I/O verified
- text-only pages load and navigate

### Milestone 3

- PNG and JPEG decoding
- FreeType text rendering
- HTTPS through the selected 3DS TLS library
- bottom-screen controls and keyboard

## Validation policy

A successful compiler invocation is not sufficient. Each milestone requires either hardware testing on a 2DS or validation in an emulator that accurately implements the relevant libctru services. Hardware remains the source of truth for framebuffer orientation, input timing, memory pressure, and networking behavior.
