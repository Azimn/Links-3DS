# Links-3DS

Links-3DS is an experimental port of the graphical Links web browser to the Nintendo 3DS family, including the original Nintendo 2DS.

The project is intentionally staged. The first hardware milestone proves the 3DS display, framebuffer orientation, input, build system, and clean shutdown before the upstream Links graphics driver is connected.

## Current milestone

Milestone 0 is a native libctru framebuffer probe:

- initializes the 3DS graphics service
- renders a deterministic test pattern on the top screen
- verifies the rotated BGR8 framebuffer layout
- reads buttons and the Circle Pad
- exits cleanly with START

Milestone 1 will add the Links 2.30 `GRDRV_3DS` graphics driver with an RGB565 software surface and the upstream bitmap, clipping, fill, line, scroll, title, device, and event contracts.

## Requirements

Install devkitPro with the 3DS development group. The build expects `DEVKITPRO` and `DEVKITARM` to be configured by the devkitPro environment.

## Build

```sh
make
```

The output is `links-3ds.3dsx`.

## Controls in the framebuffer probe

- Circle Pad moves the cursor marker
- A toggles the background test pattern
- START exits

## Upstream source

Run `scripts/fetch-links.sh` to download and unpack the pinned Links 2.30 source archive into `upstream/links-2.30`. Upstream source is not committed to this repository.

## Porting policy

The 3DS backend will be implemented as a native Links graphics driver. It will not emulate Linux `/dev/fb0`, override Links scheduler symbols, or wrap `links_main()` in a second event loop. Driver registration and configuration will be maintained as explicit patches against the pinned upstream release.

## Status

This repository currently contains a platform probe, not a functioning browser. The probe exists to remove display and input uncertainty before browser integration begins.
