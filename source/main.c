#include <3ds.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TOP_WIDTH 400
#define TOP_HEIGHT 240

static inline uint8_t expand5(uint16_t value)
{
    return (uint8_t)((value << 3) | (value >> 2));
}

static inline uint8_t expand6(uint16_t value)
{
    return (uint8_t)((value << 2) | (value >> 4));
}

static inline uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
    return (uint16_t)(((uint16_t)(red >> 3) << 11) |
                      ((uint16_t)(green >> 2) << 5) |
                      ((uint16_t)(blue >> 3)));
}

static void put_top_pixel(uint8_t *framebuffer, int x, int y, uint16_t pixel)
{
    size_t offset;

    if (framebuffer == NULL || x < 0 || x >= TOP_WIDTH ||
        y < 0 || y >= TOP_HEIGHT) {
        return;
    }

    /*
     * libctru exposes the top framebuffer as a 240 by 400 BGR8 surface.
     * The LCD appears rotated relative to the linear memory layout.
     */
    offset = ((size_t)x * TOP_HEIGHT + (size_t)(TOP_HEIGHT - 1 - y)) * 3u;

    framebuffer[offset + 0] = expand5(pixel & 0x1fu);
    framebuffer[offset + 1] = expand6((pixel >> 5) & 0x3fu);
    framebuffer[offset + 2] = expand5((pixel >> 11) & 0x1fu);
}

static void fill_test_pattern(uint8_t *framebuffer, bool alternate)
{
    int x;
    int y;

    for (y = 0; y < TOP_HEIGHT; ++y) {
        for (x = 0; x < TOP_WIDTH; ++x) {
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

            put_top_pixel(framebuffer, x, y, rgb565(red, green, blue));
        }
    }
}

static void draw_cursor(uint8_t *framebuffer, int cursor_x, int cursor_y)
{
    int delta;
    uint16_t white = rgb565(255, 255, 255);
    uint16_t black = rgb565(0, 0, 0);

    for (delta = -7; delta <= 7; ++delta) {
        put_top_pixel(framebuffer, cursor_x + delta, cursor_y, black);
        put_top_pixel(framebuffer, cursor_x, cursor_y + delta, black);
    }

    for (delta = -5; delta <= 5; ++delta) {
        put_top_pixel(framebuffer, cursor_x + delta, cursor_y, white);
        put_top_pixel(framebuffer, cursor_x, cursor_y + delta, white);
    }
}

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

int main(int argc, char **argv)
{
    bool alternate = false;
    int cursor_x = TOP_WIDTH / 2;
    int cursor_y = TOP_HEIGHT / 2;

    (void)argc;
    (void)argv;

    gfxInit(GSP_BGR8_OES, GSP_BGR8_OES, false);
    gfxSet3D(false);

    while (aptMainLoop()) {
        uint8_t *framebuffer;
        uint16_t framebuffer_width;
        uint16_t framebuffer_height;
        circlePosition circle;
        u32 down;

        hidScanInput();
        down = hidKeysDown();

        if ((down & KEY_START) != 0) {
            break;
        }

        if ((down & KEY_A) != 0) {
            alternate = !alternate;
        }

        hidCircleRead(&circle);
        cursor_x = clamp_int(cursor_x + circle.dx / 24, 0, TOP_WIDTH - 1);
        cursor_y = clamp_int(cursor_y - circle.dy / 24, 0, TOP_HEIGHT - 1);

        framebuffer = gfxGetFramebuffer(
            GFX_TOP,
            GFX_LEFT,
            &framebuffer_width,
            &framebuffer_height
        );

        if (framebuffer != NULL && framebuffer_width == TOP_HEIGHT &&
            framebuffer_height == TOP_WIDTH) {
            fill_test_pattern(framebuffer, alternate);
            draw_cursor(framebuffer, cursor_x, cursor_y);
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
