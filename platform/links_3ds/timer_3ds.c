#include "links.h"

#include "links_3ds_platform.h"

#include <stddef.h>

static struct timer *input_timer;
static int input_timer_running;

static void links_3ds_input_timer_callback(void *unused)
{
    (void)unused;

    /* Expired timers are removed by Links before their callback runs. */
    input_timer = NULL;

    if (!input_timer_running) {
        return;
    }

    links_3ds_platform_poll();

    input_timer = install_timer(LINKS_3DS_INPUT_INTERVAL_MS,
                                links_3ds_input_timer_callback,
                                NULL);
}

void links_3ds_platform_start_input_timer(void)
{
    if (input_timer != NULL) {
        kill_timer(input_timer);
        input_timer = NULL;
    }

    input_timer_running = 1;
    input_timer = install_timer(LINKS_3DS_INPUT_INTERVAL_MS,
                                links_3ds_input_timer_callback,
                                NULL);
}

void links_3ds_platform_stop_input_timer(void)
{
    input_timer_running = 0;

    if (input_timer != NULL) {
        kill_timer(input_timer);
        input_timer = NULL;
    }
}
