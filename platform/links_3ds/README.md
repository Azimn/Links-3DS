# Links 3DS platform adapter

This directory contains the platform boundary between Links 2.30 and the native 3DS layer in `source/gfx_3ds.*`.

Milestone 1A deliberately keeps the adapter separate from the upstream browser checkout. Files here must not redefine Links scheduler functions, replace its event loop, or emulate Linux framebuffer APIs. The final integration will compile these files with the exact Links 2.30 headers after `scripts/fetch-links.sh` has downloaded and verified the pinned source archive.

The adapter is split into four responsibilities:

- `links_3ds_platform.h` defines neutral platform-facing types and callbacks.
- `input_3ds.c` translates `gfx_input_t` into browser-neutral keyboard and pointer events.
- `timer_3ds.c` provides a one-shot polling callback that the Links scheduler can re-arm.
- `platform_3ds.c` owns initialization, shutdown, cursor state, and the active event sink.

The actual `struct graphics_driver links_3ds_driver` implementation will be added only after the build consumes the verified Links 2.30 `links.h`. This avoids freezing guessed callback signatures into the repository.
