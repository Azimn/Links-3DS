#ifndef LINKS_3DS_GFX_3DS_H
#define LINKS_3DS_GFX_3DS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct gfx_surface gfx_surface_t;

bool gfx_3ds_init(void);
void gfx_3ds_exit(void);

gfx_surface_t *gfx_surface_create(int width, int height);
void gfx_surface_destroy(gfx_surface_t *surface);

int gfx_surface_width(const gfx_surface_t *surface);
int gfx_surface_height(const gfx_surface_t *surface);
int gfx_surface_stride_bytes(const gfx_surface_t *surface);

uint16_t *gfx_surface_lock(gfx_surface_t *surface);
void gfx_surface_unlock(gfx_surface_t *surface);

void gfx_set_clip(gfx_surface_t *surface, int x1, int y1, int x2, int y2);
void gfx_get_clip(const gfx_surface_t *surface, int *x1, int *y1, int *x2, int *y2);

void gfx_fill_rect(gfx_surface_t *surface, int x1, int y1, int x2, int y2,
                   uint16_t color);
void gfx_draw_hline(gfx_surface_t *surface, int x1, int x2, int y,
                    uint16_t color);
void gfx_draw_vline(gfx_surface_t *surface, int y1, int y2, int x,
                    uint16_t color);
void gfx_draw_bitmap(gfx_surface_t *destination, int destination_x,
                     int destination_y, const gfx_surface_t *source,
                     int source_x, int source_y, int width, int height);

void gfx_present(const gfx_surface_t *surface);

typedef enum gfx_key {
    GFX_KEY_A = 1u << 0,
    GFX_KEY_B = 1u << 1,
    GFX_KEY_X = 1u << 2,
    GFX_KEY_Y = 1u << 3,
    GFX_KEY_START = 1u << 4,
    GFX_KEY_SELECT = 1u << 5,
    GFX_KEY_UP = 1u << 6,
    GFX_KEY_DOWN = 1u << 7,
    GFX_KEY_LEFT = 1u << 8,
    GFX_KEY_RIGHT = 1u << 9,
    GFX_KEY_L = 1u << 10,
    GFX_KEY_R = 1u << 11
} gfx_key_t;

typedef struct gfx_input {
    uint32_t keys_down;
    uint32_t keys_held;
    uint32_t keys_up;
    int touch_x;
    int touch_y;
    int cpad_dx;
    int cpad_dy;
} gfx_input_t;

void gfx_poll_input(gfx_input_t *input);

void gfx_set_cursor_pos(int x, int y);
void gfx_get_cursor_pos(int *x, int *y);
void gfx_set_cursor_visible(bool visible);

uint16_t gfx_rgb565(uint8_t red, uint8_t green, uint8_t blue);

#endif
