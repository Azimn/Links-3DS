#include "links_3ds_platform.h"

static links_3ds_schedule_fn schedule_callback;

void links_3ds_platform_set_scheduler(links_3ds_schedule_fn scheduler)
{
    schedule_callback = scheduler;
}

void links_3ds_platform_start_input_timer(void)
{
    if (schedule_callback != NULL) {
        schedule_callback(LINKS_3DS_INPUT_INTERVAL_MS,
                          links_3ds_platform_timer_callback,
                          NULL);
    }
}

void links_3ds_platform_timer_callback(void *unused)
{
    (void)unused;

    links_3ds_platform_poll();
    links_3ds_platform_start_input_timer();
}
