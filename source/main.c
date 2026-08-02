#include <3ds.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gfx_3ds.h"

#define TOP_WIDTH 400
#define TOP_HEIGHT 240

static int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static void fill_test_pattern(gfx_surface_t *surface, bool alternate)
{
    uint16_t *pixels;
    int stride_pixels;
    int x;
    int y;

    pixels = gfx_surface_lock(surface);
    if (pixels == NULL) {
        return;
    }

    stride_pixels = gfx_surface_stride_bytes(surface) / (int)sizeof(uint16_t);

    for (y = 0; y < gfx_surface_height(surface); ++y) {
        for (x = 0; x < gfx_surface_width(surface); ++x) {
            uint8_t red;
            uint8_t green;
            uint8_t blue;

            if (alternate) {
                red = (uint8_t)((x * 255) / (TOP_WIDTH - 1));
                green = (uint8_t)((y * 255) / (TOP_HEIGHT - 1));
                blue = (uint8_t)(((x / 20 + y / 20) & 1) ? 220 : 24);
            } else {
                red = (uint8_t)(((x / 32) & 1) ? 230 : 24);
                green = (uint8_t)(((y / 24) & 1) ? 210 : 32);
                blue = (uint8_t)((x * 180) / (TOP_WIDTH - 1));
            }

            pixels[(size_t)y * (size_t)stride_pixels + (size_t)x] =
                gfx_rgb565(red, green, blue);
        }
    }

    gfx_surface_unlock(surface);

    gfx_set_clip(surface, 12, 12, TOP_WIDTH - 12, TOP_HEIGHT - 12);
    gfx_draw_hline(surface, 12, TOP_WIDTH - 12, 12,
                   gfx_rgb565(255, 255, 255));
    gfx_draw_hline(surface, 12, TOP_WIDTH - 12, TOP_HEIGHT - 13,
                   gfx_rgb565(255, 255, 255));
    gfx_draw_vline(surface, 12, TOP_HEIGHT - 12, 12,
                   gfx_rgb565(255, 255, 255));
    gfx_draw_vline(surface, 12, TOP_HEIGHT - 12, TOP_WIDTH - 13,
                   gfx_rgb565(255, 255, 255));
    gfx_set_clip(surface, 0, 0, TOP_WIDTH, TOP_HEIGHT);
}

int main(int argc, char **argv)
{
    gfx_surface_t *surface;
    gfx_input_t input;
    bool alternate = false;
    int cursor_x = TOP_WIDTH / 2;
    int cursor_y = TOP_HEIGHT / 2;

    (void)argc;
    (void)argv;

    if (!gfx_3ds_init()) {
        return 1;
    }

    surface = gfx_surface_create(TOP_WIDTH, TOP_HEIGHT);
    if (surface == NULL) {
        gfx_3ds_exit();
        return 1;
    }

    gfx_set_cursor_pos(cursor_x, cursor_y);
    gfx_set_cursor_visible(true);

    while (aptMainLoop()) {
        gfx_poll_input(&input);

        if ((input.keys_down & GFX_KEY_START) != 0) {
            break;
        }

        if ((input.keys_down & GFX_KEY_A) != 0) {
            alternate = !alternate;
        }

        cursor_x = clamp_int(cursor_x + input.cpad_dx / 24, 0,
                             TOP_WIDTH - 1);
        cursor_y = clamp_int(cursor_y - input.cpad_dy / 24, 0,
                             TOP_HEIGHT - 1);
        gfx_set_cursor_pos(cursor_x, cursor_y);

        fill_test_pattern(surface, alternate);
        gfx_present(surface);
    }

    gfx_set_cursor_visible(false);
    gfx_surface_destroy(surface);
    gfx_3ds_exit();
    return 0;
}
