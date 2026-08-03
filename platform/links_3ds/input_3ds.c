#include "links_3ds_platform.h"
#include "browser_ui_3ds.h"

#include <3ds.h>

#define LINKS_3DS_POINTER_PRIMARY 1u
#define LINKS_3DS_CPAD_DIVISOR 24

static bool touch_was_held;
static int last_touch_x;
static int last_touch_y;

static void emit_key(const links_3ds_event_sink_t *sink,
                     uint32_t keys_down,
                     uint32_t mask,
                     links_3ds_key_t key)
{
    if ((keys_down & mask) != 0u && sink->keyboard != NULL) {
        sink->keyboard(sink->context, key, 0u);
    }
}

void links_3ds_platform_poll(void)
{
    const links_3ds_event_sink_t *sink = links_3ds_platform_get_event_sink();
    gfx_input_t input;
    int cursor_x;
    int cursor_y;
    int next_x;
    int next_y;
    bool moved;
    bool touch_held;
    bool touch_down;
    bool touch_up;

    if (!aptMainLoop()) {
        if (sink->keyboard != NULL) {
            sink->keyboard(sink->context, LINKS_3DS_KEY_CLOSE, 0u);
        }
        return;
    }

    gfx_poll_input(&input);

    touch_held = input.touch_x >= 0 && input.touch_y >= 0;
    touch_down = touch_held && !touch_was_held;
    touch_up = !touch_held && touch_was_held;
    if (touch_held) {
        last_touch_x = input.touch_x;
        last_touch_y = input.touch_y;
    }
    if (touch_down || touch_held || touch_up) {
        if (links_3ds_ui_handle_touch(last_touch_x, last_touch_y,
                                      touch_down, touch_held, touch_up, sink)) {
            touch_was_held = touch_held;
            return;
        }
    }
    touch_was_held = touch_held;

    if ((input.keys_down & GFX_KEY_SELECT) != 0u) {
        emit_key(sink, GFX_KEY_SELECT, GFX_KEY_SELECT, LINKS_3DS_KEY_GOTO_URL);
        links_3ds_ui_open_keyboard(sink, NULL,
                                   "Enter web address or search", false);
        return;
    }

    gfx_get_cursor_pos(&cursor_x, &cursor_y);

    next_x = cursor_x + input.cpad_dx / LINKS_3DS_CPAD_DIVISOR;
    next_y = cursor_y - input.cpad_dy / LINKS_3DS_CPAD_DIVISOR;

    if (next_x < 0) {
        next_x = 0;
    } else if (next_x >= LINKS_3DS_SCREEN_WIDTH) {
        next_x = LINKS_3DS_SCREEN_WIDTH - 1;
    }

    if (next_y < 0) {
        next_y = 0;
    } else if (next_y >= LINKS_3DS_SCREEN_HEIGHT) {
        next_y = LINKS_3DS_SCREEN_HEIGHT - 1;
    }

    moved = next_x != cursor_x || next_y != cursor_y;
    if (moved) {
        gfx_set_cursor_pos(next_x, next_y);
        if (sink->pointer != NULL) {
            links_3ds_pointer_action_t action =
                (input.keys_held & GFX_KEY_A) != 0u
                    ? LINKS_3DS_POINTER_DRAG
                    : LINKS_3DS_POINTER_MOVE;
            sink->pointer(sink->context, next_x, next_y, action,
                          (input.keys_held & GFX_KEY_A) != 0u
                              ? LINKS_3DS_POINTER_PRIMARY
                              : 0u);
        }
    }

    if (sink->pointer != NULL && (input.keys_down & GFX_KEY_A) != 0u) {
        sink->pointer(sink->context, next_x, next_y,
                      LINKS_3DS_POINTER_DOWN, LINKS_3DS_POINTER_PRIMARY);
    }

    if (sink->pointer != NULL && (input.keys_up & GFX_KEY_A) != 0u) {
        sink->pointer(sink->context, next_x, next_y,
                      LINKS_3DS_POINTER_UP, LINKS_3DS_POINTER_PRIMARY);
    }

    emit_key(sink, input.keys_down, GFX_KEY_UP, LINKS_3DS_KEY_UP);
    emit_key(sink, input.keys_down, GFX_KEY_DOWN, LINKS_3DS_KEY_DOWN);
    emit_key(sink, input.keys_down, GFX_KEY_LEFT, LINKS_3DS_KEY_LEFT);
    emit_key(sink, input.keys_down, GFX_KEY_RIGHT, LINKS_3DS_KEY_RIGHT);
    emit_key(sink, input.keys_down, GFX_KEY_A, LINKS_3DS_KEY_ENTER);
    emit_key(sink, input.keys_down, GFX_KEY_B, LINKS_3DS_KEY_BACK);
    emit_key(sink, input.keys_down, GFX_KEY_X, LINKS_3DS_KEY_ESCAPE);
    emit_key(sink, input.keys_down, GFX_KEY_Y, LINKS_3DS_KEY_RELOAD);
    emit_key(sink, input.keys_down, GFX_KEY_L, LINKS_3DS_KEY_PAGE_UP);
    emit_key(sink, input.keys_down, GFX_KEY_R, LINKS_3DS_KEY_PAGE_DOWN);
    emit_key(sink, input.keys_down, GFX_KEY_START, LINKS_3DS_KEY_CLOSE);
}
