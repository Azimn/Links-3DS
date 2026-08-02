#include "links_3ds_platform.h"

#include <string.h>

static links_3ds_event_sink_t active_sink;
static bool platform_ready;

bool links_3ds_platform_init(void)
{
    if (platform_ready) {
        return true;
    }

    if (!gfx_3ds_init()) {
        return false;
    }

    memset(&active_sink, 0, sizeof(active_sink));
    gfx_set_cursor_pos(LINKS_3DS_SCREEN_WIDTH / 2,
                       LINKS_3DS_SCREEN_HEIGHT / 2);
    gfx_set_cursor_visible(true);
    platform_ready = true;
    return true;
}

void links_3ds_platform_shutdown(void)
{
    if (!platform_ready) {
        return;
    }

    memset(&active_sink, 0, sizeof(active_sink));
    gfx_3ds_exit();
    platform_ready = false;
}

void links_3ds_platform_set_event_sink(const links_3ds_event_sink_t *sink)
{
    if (sink == NULL) {
        memset(&active_sink, 0, sizeof(active_sink));
        return;
    }

    active_sink = *sink;
}

const links_3ds_event_sink_t *links_3ds_platform_get_event_sink(void)
{
    return &active_sink;
}
