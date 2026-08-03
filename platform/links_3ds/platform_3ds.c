#include "links_3ds_platform.h"
#include "browser_ui_3ds.h"

#include <3ds.h>

#include <string.h>

#define LINKS_3DS_SOC_BUFFER_SIZE (1024u * 1024u)

static links_3ds_event_sink_t active_sink;
static bool platform_ready;
static u32 *soc_buffer;

bool links_3ds_platform_init(void)
{
    Result result;

    if (platform_ready) {
        return true;
    }

    if (!gfx_3ds_init()) {
        return false;
    }

    if (!links_3ds_ui_init()) {
        gfx_3ds_exit();
        return false;
    }

    soc_buffer = (u32 *)linearAlloc(LINKS_3DS_SOC_BUFFER_SIZE);
    if (soc_buffer == NULL) {
        links_3ds_ui_shutdown();
        gfx_3ds_exit();
        return false;
    }

    result = socInit(soc_buffer, LINKS_3DS_SOC_BUFFER_SIZE);
    if (R_FAILED(result)) {
        linearFree(soc_buffer);
        soc_buffer = NULL;
        links_3ds_ui_shutdown();
        gfx_3ds_exit();
        return false;
    }

    memset(&active_sink, 0, sizeof(active_sink));
    gfx_set_cursor_pos(LINKS_3DS_SCREEN_WIDTH / 2,
                       LINKS_3DS_SCREEN_HEIGHT / 2);
    gfx_set_cursor_visible(true);
    links_3ds_ui_set_status("Network ready");
    links_3ds_ui_render();
    platform_ready = true;
    return true;
}

void links_3ds_platform_shutdown(void)
{
    if (!platform_ready) {
        return;
    }

    links_3ds_platform_stop_input_timer();
    memset(&active_sink, 0, sizeof(active_sink));

    socExit();
    if (soc_buffer != NULL) {
        linearFree(soc_buffer);
        soc_buffer = NULL;
    }

    links_3ds_ui_shutdown();
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
