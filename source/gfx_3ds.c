#include "gfx_3ds.h"

#include <3ds.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240

struct gfx_surface {
    uint16_t *pixels;
    int width;
    int height;
    int stride_pixels;
    int clip_x1;
    int clip_y1;
    int clip_x2;
    int clip_y2;
    bool locked;
};

static bool graphics_initialized;
static int cursor_x = TOP_SCREEN_WIDTH / 2;
static int cursor_y = TOP_SCREEN_HEIGHT / 2;
static bool cursor_visible;

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

static uint8_t expand5(uint16_t value)
{
    return (uint8_t)((value << 3) | (value >> 2));
}

static uint8_t expand6(uint16_t value)
{
    return (uint8_t)((value << 2) | (value >> 4));
}

static uint32_t translate_keys(u32 keys)
{
    uint32_t translated = 0;

    if ((keys & KEY_A) != 0) {
        translated |= GFX_KEY_A;
    }
    if ((keys & KEY_B) != 0) {
        translated |= GFX_KEY_B;
    }
    if ((keys & KEY_X) != 0) {
        translated |= GFX_KEY_X;
    }
    if ((keys & KEY_Y) != 0) {
        translated |= GFX_KEY_Y;
    }
    if ((keys & KEY_START) != 0) {
        translated |= GFX_KEY_START;
    }
    if ((keys & KEY_SELECT) != 0) {
        translated |= GFX_KEY_SELECT;
    }
    if ((keys & KEY_DUP) != 0) {
        translated |= GFX_KEY_UP;
    }
    if ((keys & KEY_DDOWN) != 0) {
        translated |= GFX_KEY_DOWN;
    }
    if ((keys & KEY_DLEFT) != 0) {
        translated |= GFX_KEY_LEFT;
    }
    if ((keys & KEY_DRIGHT) != 0) {
        translated |= GFX_KEY_RIGHT;
    }
    if ((keys & KEY_L) != 0) {
        translated |= GFX_KEY_L;
    }
    if ((keys & KEY_R) != 0) {
        translated |= GFX_KEY_R;
    }

    return translated;
}

static void write_top_pixel(uint8_t *framebuffer, int x, int y, uint16_t pixel)
{
    size_t offset;

    if (framebuffer == NULL || x < 0 || x >= TOP_SCREEN_WIDTH ||
        y < 0 || y >= TOP_SCREEN_HEIGHT) {
        return;
    }

    offset = ((size_t)x * TOP_SCREEN_HEIGHT +
              (size_t)(TOP_SCREEN_HEIGHT - 1 - y)) * 3u;

    framebuffer[offset + 0] = expand5(pixel & 0x1fu);
    framebuffer[offset + 1] = expand6((pixel >> 5) & 0x3fu);
    framebuffer[offset + 2] = expand5((pixel >> 11) & 0x1fu);
}

static void draw_present_cursor(uint8_t *framebuffer)
{
    int delta;
    uint16_t black = gfx_rgb565(0, 0, 0);
    uint16_t white = gfx_rgb565(255, 255, 255);

    if (!cursor_visible) {
        return;
    }

    for (delta = -7; delta <= 7; ++delta) {
        write_top_pixel(framebuffer, cursor_x + delta, cursor_y, black);
        write_top_pixel(framebuffer, cursor_x, cursor_y + delta, black);
    }

    for (delta = -5; delta <= 5; ++delta) {
        write_top_pixel(framebuffer, cursor_x + delta, cursor_y, white);
        write_top_pixel(framebuffer, cursor_x, cursor_y + delta, white);
    }
}

bool gfx_3ds_init(void)
{
    if (graphics_initialized) {
        return true;
    }

    gfxInit(GSP_BGR8_OES, GSP_BGR8_OES, false);
    gfxSet3D(false);
    graphics_initialized = true;
    cursor_visible = false;
    return true;
}

void gfx_3ds_exit(void)
{
    if (!graphics_initialized) {
        return;
    }

    gfxExit();
    graphics_initialized = false;
}

gfx_surface_t *gfx_surface_create(int width, int height)
{
    gfx_surface_t *surface;
    size_t pixel_count;

    if (width <= 0 || height <= 0) {
        return NULL;
    }

    surface = calloc(1, sizeof(*surface));
    if (surface == NULL) {
        return NULL;
    }

    pixel_count = (size_t)width * (size_t)height;
    surface->pixels = linearAlloc(pixel_count * sizeof(*surface->pixels));
    if (surface->pixels == NULL) {
        free(surface);
        return NULL;
    }

    memset(surface->pixels, 0, pixel_count * sizeof(*surface->pixels));
    surface->width = width;
    surface->height = height;
    surface->stride_pixels = width;
    surface->clip_x1 = 0;
    surface->clip_y1 = 0;
    surface->clip_x2 = width;
    surface->clip_y2 = height;
    return surface;
}

void gfx_surface_destroy(gfx_surface_t *surface)
{
    if (surface == NULL) {
        return;
    }

    if (surface->pixels != NULL) {
        linearFree(surface->pixels);
    }
    free(surface);
}

int gfx_surface_width(const gfx_surface_t *surface)
{
    return surface != NULL ? surface->width : 0;
}

int gfx_surface_height(const gfx_surface_t *surface)
{
    return surface != NULL ? surface->height : 0;
}

int gfx_surface_stride_bytes(const gfx_surface_t *surface)
{
    return surface != NULL ? surface->stride_pixels * (int)sizeof(uint16_t) : 0;
}

uint16_t *gfx_surface_lock(gfx_surface_t *surface)
{
    if (surface == NULL) {
        return NULL;
    }
    surface->locked = true;
    return surface->pixels;
}

void gfx_surface_unlock(gfx_surface_t *surface)
{
    if (surface != NULL) {
        surface->locked = false;
    }
}

void gfx_set_clip(gfx_surface_t *surface, int x1, int y1, int x2, int y2)
{
    if (surface == NULL) {
        return;
    }

    surface->clip_x1 = clamp_int(x1, 0, surface->width);
    surface->clip_y1 = clamp_int(y1, 0, surface->height);
    surface->clip_x2 = clamp_int(x2, surface->clip_x1, surface->width);
    surface->clip_y2 = clamp_int(y2, surface->clip_y1, surface->height);
}

void gfx_get_clip(const gfx_surface_t *surface, int *x1, int *y1, int *x2, int *y2)
{
    if (surface == NULL) {
        return;
    }

    if (x1 != NULL) {
        *x1 = surface->clip_x1;
    }
    if (y1 != NULL) {
        *y1 = surface->clip_y1;
    }
    if (x2 != NULL) {
        *x2 = surface->clip_x2;
    }
    if (y2 != NULL) {
        *y2 = surface->clip_y2;
    }
}

void gfx_fill_rect(gfx_surface_t *surface, int x1, int y1, int x2, int y2,
                   uint16_t color)
{
    int x;
    int y;

    if (surface == NULL || surface->pixels == NULL) {
        return;
    }

    x1 = clamp_int(x1, surface->clip_x1, surface->clip_x2);
    y1 = clamp_int(y1, surface->clip_y1, surface->clip_y2);
    x2 = clamp_int(x2, x1, surface->clip_x2);
    y2 = clamp_int(y2, y1, surface->clip_y2);

    for (y = y1; y < y2; ++y) {
        uint16_t *row = surface->pixels + (size_t)y * (size_t)surface->stride_pixels;
        for (x = x1; x < x2; ++x) {
            row[x] = color;
        }
    }
}

void gfx_draw_hline(gfx_surface_t *surface, int x1, int x2, int y,
                    uint16_t color)
{
    gfx_fill_rect(surface, x1, y, x2, y + 1, color);
}

void gfx_draw_vline(gfx_surface_t *surface, int y1, int y2, int x,
                    uint16_t color)
{
    gfx_fill_rect(surface, x, y1, x + 1, y2, color);
}

void gfx_draw_bitmap(gfx_surface_t *destination, int destination_x,
                     int destination_y, const gfx_surface_t *source,
                     int source_x, int source_y, int width, int height)
{
    int row;

    if (destination == NULL || source == NULL || destination->pixels == NULL ||
        source->pixels == NULL || width <= 0 || height <= 0) {
        return;
    }

    if (source_x < 0) {
        destination_x -= source_x;
        width += source_x;
        source_x = 0;
    }
    if (source_y < 0) {
        destination_y -= source_y;
        height += source_y;
        source_y = 0;
    }
    if (destination_x < destination->clip_x1) {
        source_x += destination->clip_x1 - destination_x;
        width -= destination->clip_x1 - destination_x;
        destination_x = destination->clip_x1;
    }
    if (destination_y < destination->clip_y1) {
        source_y += destination->clip_y1 - destination_y;
        height -= destination->clip_y1 - destination_y;
        destination_y = destination->clip_y1;
    }

    width = clamp_int(width, 0, source->width - source_x);
    height = clamp_int(height, 0, source->height - source_y);
    width = clamp_int(width, 0, destination->clip_x2 - destination_x);
    height = clamp_int(height, 0, destination->clip_y2 - destination_y);

    for (row = 0; row < height; ++row) {
        const uint16_t *source_row = source->pixels +
            (size_t)(source_y + row) * (size_t)source->stride_pixels + source_x;
        uint16_t *destination_row = destination->pixels +
            (size_t)(destination_y + row) * (size_t)destination->stride_pixels +
            destination_x;
        memmove(destination_row, source_row, (size_t)width * sizeof(uint16_t));
    }
}

void gfx_present(const gfx_surface_t *surface)
{
    uint8_t *framebuffer;
    uint16_t framebuffer_width;
    uint16_t framebuffer_height;
    int x;
    int y;
    int draw_width;
    int draw_height;

    if (!graphics_initialized || surface == NULL || surface->pixels == NULL) {
        return;
    }

    framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &framebuffer_width,
                                    &framebuffer_height);
    if (framebuffer == NULL || framebuffer_width != TOP_SCREEN_HEIGHT ||
        framebuffer_height != TOP_SCREEN_WIDTH) {
        return;
    }

    draw_width = clamp_int(surface->width, 0, TOP_SCREEN_WIDTH);
    draw_height = clamp_int(surface->height, 0, TOP_SCREEN_HEIGHT);

    for (y = 0; y < draw_height; ++y) {
        const uint16_t *row = surface->pixels +
            (size_t)y * (size_t)surface->stride_pixels;
        for (x = 0; x < draw_width; ++x) {
            write_top_pixel(framebuffer, x, y, row[x]);
        }
    }

    draw_present_cursor(framebuffer);
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
}

void gfx_poll_input(gfx_input_t *input)
{
    circlePosition circle;
    touchPosition touch;
    u32 held;

    if (input == NULL) {
        return;
    }

    hidScanInput();
    held = hidKeysHeld();

    input->keys_down = translate_keys(hidKeysDown());
    input->keys_held = translate_keys(held);
    input->keys_up = translate_keys(hidKeysUp());
    input->touch_x = -1;
    input->touch_y = -1;

    hidCircleRead(&circle);
    input->cpad_dx = circle.dx;
    input->cpad_dy = circle.dy;

    if ((held & KEY_TOUCH) != 0) {
        touchRead(&touch);
        input->touch_x = touch.px;
        input->touch_y = touch.py;
    }
}

void gfx_set_cursor_pos(int x, int y)
{
    cursor_x = clamp_int(x, 0, TOP_SCREEN_WIDTH - 1);
    cursor_y = clamp_int(y, 0, TOP_SCREEN_HEIGHT - 1);
}

void gfx_get_cursor_pos(int *x, int *y)
{
    if (x != NULL) {
        *x = cursor_x;
    }
    if (y != NULL) {
        *y = cursor_y;
    }
}

void gfx_set_cursor_visible(bool visible)
{
    cursor_visible = visible;
}

uint16_t gfx_rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
    return (uint16_t)(((uint16_t)(red >> 3) << 11) |
                      ((uint16_t)(green >> 2) << 5) |
                      (uint16_t)(blue >> 3));
}
