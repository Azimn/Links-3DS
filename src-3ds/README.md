# Links 2.30 3DS driver staging

`graphics_3ds.c` is the first source-accurate driver implementation derived from the verified Links 2.30 interface report.

The file is intentionally outside the current probe source directory. The confirmed Milestone 0.5 build remains unchanged while the driver is checked against the full upstream configuration and scheduler interfaces.

Corrections already incorporated:

* RGB565 depth encoding uses two bytes per pixel and sixteen significant bits.
* Screen margins are zero on all sides.
* Unsupported title, clipboard, command execution, palette, and Unix socket callbacks are `NULL`.
* The driver advertises one window, no operating system shell, and no libevent dependency.
* Bitmap copies lock the surface once and use the surface byte stride.
* The initial scroll implementation requests redraw rather than performing an unsafe partial copy.
* The driver uses root-level Links 2.30 headers and source layout.

Before this driver is linked into the browser build, CI must complete these checks:

1. Apply `patches/0001-register-3ds-driver.patch` cleanly to the verified archive.
2. Generate a 3DS-specific `cfg.h` with `G` and `GRDRV_3DS` enabled.
3. Compile this file against the extracted Links 2.30 headers with warnings treated as errors.
4. Confirm the exact bitmap flag type and memory allocation behavior.
5. Bind input polling to the verified Links timer API.
6. Add this file and the platform adapter to the explicit upstream object list.

The existing probe remains the release artifact until these checks pass.
