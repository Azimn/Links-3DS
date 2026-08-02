#!/usr/bin/env python3
"""Apply the verified Links 2.30 driver integration to graphics_3ds.c.

The repository keeps the staged driver readable, while CI and local full builds
need the designated Links 2.30 initializer, platform services, and input
lifecycle hooks. This transformation is deterministic and idempotent.
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "src-3ds" / "graphics_3ds.c"

source = PATH.read_text(encoding="utf-8")

platform_include = '#include "../platform/links_3ds/links_3ds_platform.h"\n'
gfx_include = '#include "../source/gfx_3ds.h"\n'
if platform_include not in source:
    if gfx_include not in source:
        raise SystemExit("gfx_3ds.h include anchor not found")
    source = source.replace(
        gfx_include,
        gfx_include + platform_include,
        1,
    )

source = source.replace(
    "    if (!gfx_3ds_init()) {\n",
    "    if (!links_3ds_platform_init()) {\n",
    1,
)
source = source.replace(
    "    gfx_3ds_exit();\n    links_3ds_initialized = 0;\n",
    "    links_3ds_platform_shutdown();\n    links_3ds_initialized = 0;\n",
    1,
)

attach_block = (
    "    links_3ds_event_bridge_attach(dev);\n"
    "    links_3ds_platform_start_input_timer();\n\n"
)
init_anchor = "    return dev;\n}\n\nstatic void links_3ds_shutdown_device"
if attach_block not in source:
    if init_anchor not in source:
        raise SystemExit("device initialization anchor not found")
    source = source.replace(
        init_anchor,
        attach_block + init_anchor,
        1,
    )

shutdown_block = (
    "    links_3ds_platform_stop_input_timer();\n"
    "    links_3ds_event_bridge_detach(dev);\n\n"
)
shutdown_anchor = "    private_data = links_3ds_private(dev);"
if shutdown_block not in source:
    if shutdown_anchor not in source:
        raise SystemExit("device shutdown anchor not found")
    source = source.replace(
        shutdown_anchor,
        shutdown_block + shutdown_anchor,
        1,
    )

marker = "struct graphics_driver links_3ds_driver = {"
start = source.find(marker)
if start < 0:
    raise SystemExit("graphics driver initializer not found")

initializer = r'''struct graphics_driver links_3ds_driver = {
    .name = cast_uchar "3ds",
    .init_driver = links_3ds_init_driver,
    .init_device = links_3ds_init_device,
    .shutdown_device = links_3ds_shutdown_device,
    .shutdown_driver = links_3ds_shutdown_driver,
    .emergency_shutdown = links_3ds_emergency_shutdown,
    .after_fork = links_3ds_after_fork,
    .get_driver_param = links_3ds_get_driver_param,
    .get_af_unix_name = NULL,
    .get_margin = links_3ds_get_margin,
    .set_margin = links_3ds_set_margin,
    .get_empty_bitmap = links_3ds_get_empty_bitmap,
    .register_bitmap = links_3ds_register_bitmap,
    .prepare_strip = links_3ds_prepare_strip,
    .commit_strip = links_3ds_commit_strip,
    .unregister_bitmap = links_3ds_unregister_bitmap,
    .draw_bitmap = links_3ds_draw_bitmap,
    .get_color = links_3ds_get_color,
    .fill_area = links_3ds_fill_area,
    .draw_hline = links_3ds_draw_hline,
    .draw_vline = links_3ds_draw_vline,
    .scroll = links_3ds_scroll,
    .set_clip_area = links_3ds_set_clip_area,
    .flush = links_3ds_flush,
    .block = links_3ds_block,
    .unblock = links_3ds_unblock,
    .set_palette = NULL,
    .get_real_colors = NULL,
    .set_title = NULL,
    .exec = NULL,
    .set_clipboard_text = NULL,
    .get_clipboard_text = NULL,
    .depth = LINKS_3DS_DEPTH,
    .x = LINKS_3DS_WIDTH,
    .y = LINKS_3DS_HEIGHT,
    .flags = GD_ONLY_1_WINDOW | GD_NO_OS_SHELL | GD_NO_LIBEVENT,
    .param = NULL
};
'''

source = source[:start] + initializer
PATH.write_text(source, encoding="utf-8")

print(f"Prepared {PATH.relative_to(ROOT)}")
