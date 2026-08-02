#ifndef LINKS_3DS_PLATFORM_H
#define LINKS_3DS_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

#include "../../source/gfx_3ds.h"

#define LINKS_3DS_SCREEN_WIDTH 400
#define LINKS_3DS_SCREEN_HEIGHT 240
#define LINKS_3DS_INPUT_INTERVAL_MS 16

typedef enum links_3ds_key {
    LINKS_3DS_KEY_NONE = 0,
    LINKS_3DS_KEY_UP,
    LINKS_3DS_KEY_DOWN,
    LINKS_3DS_KEY_LEFT,
    LINKS_3DS_KEY_RIGHT,
    LINKS_3DS_KEY_ENTER,
    LINKS_3DS_KEY_ESCAPE,
    LINKS_3DS_KEY_BACK,
    LINKS_3DS_KEY_RELOAD,
    LINKS_3DS_KEY_PAGE_UP,
    LINKS_3DS_KEY_PAGE_DOWN,
    LINKS_3DS_KEY_CLOSE
} links_3ds_key_t;

typedef enum links_3ds_pointer_action {
    LINKS_3DS_POINTER_MOVE = 0,
    LINKS_3DS_POINTER_DOWN,
    LINKS_3DS_POINTER_UP,
    LINKS_3DS_POINTER_DRAG
} links_3ds_pointer_action_t;

typedef struct links_3ds_event_sink {
    void *context;
    void (*keyboard)(void *context, links_3ds_key_t key, uint32_t modifiers);
    void (*pointer)(void *context,
                    int x,
                    int y,
                    links_3ds_pointer_action_t action,
                    uint32_t buttons);
} links_3ds_event_sink_t;

bool links_3ds_platform_init(void);
void links_3ds_platform_shutdown(void);
void links_3ds_platform_set_event_sink(const links_3ds_event_sink_t *sink);
const links_3ds_event_sink_t *links_3ds_platform_get_event_sink(void);
void links_3ds_platform_start_input_timer(void);
void links_3ds_platform_stop_input_timer(void);
void links_3ds_platform_poll(void);

#endif
