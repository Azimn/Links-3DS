#include "links.h"

#include "../source/gfx_3ds.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define LINKS_3DS_WIDTH 400
#define LINKS_3DS_HEIGHT 240
#define LINKS_3DS_DEPTH (2 | (16 << 3))

struct links_3ds_device {
    gfx_surface_t *surface;
};

static int links_3ds_initialized;

static struct links_3ds_device *links_3ds_private(struct graphics_device *dev)
{
    if (dev == NULL) {
        return NULL;
    }
    return (struct links_3ds_device *)dev->driver_data;
}

static unsigned char *links_3ds_init_driver(unsigned char *param,
                                             unsigned char *display)
{
    (void)param;
    (void)display;

    if (!gfx_3ds_init()) {
        return cast_uchar "Unable to initialize 3DS graphics";
    }

    links_3ds_initialized = 1;
    return NULL;
}

static struct graphics_device *links_3ds_init_device(void)
{
    struct graphics_device *dev;
    struct links_3ds_device *private_data;

    if (!links_3ds_initialized) {
        return NULL;
    }

    dev = (struct graphics_device *)mem_alloc(sizeof(*dev));
    memset(dev, 0, sizeof(*dev));

    private_data = (struct links_3ds_device *)mem_alloc(sizeof(*private_data));
    memset(private_data, 0, sizeof(*private_data));

    private_data->surface = gfx_surface_create(LINKS_3DS_WIDTH,
                                                LINKS_3DS_HEIGHT);
    if (private_data->surface == NULL) {
        mem_free(private_data);
        mem_free(dev);
        return NULL;
    }

    dev->size.x1 = 0;
    dev->size.y1 = 0;
    dev->size.x2 = LINKS_3DS_WIDTH;
    dev->size.y2 = LINKS_3DS_HEIGHT;
    dev->clip = dev->size;
    dev->driver_data = private_data;

    gfx_set_clip(private_data->surface,
                 dev->clip.x1,
                 dev->clip.y1,
                 dev->clip.x2,
                 dev->clip.y2);

    return dev;
}

static void links_3ds_shutdown_device(struct graphics_device *dev)
{
    struct links_3ds_device *private_data;

    if (dev == NULL) {
        return;
    }

    private_data = links_3ds_private(dev);
    if (private_data != NULL) {
        gfx_surface_destroy(private_data->surface);
        mem_free(private_data);
    }

    mem_free(dev);
}

static void links_3ds_shutdown_driver(void)
{
    if (!links_3ds_initialized) {
        return;
    }

    gfx_3ds_exit();
    links_3ds_initialized = 0;
}

static void links_3ds_emergency_shutdown(void)
{
    links_3ds_shutdown_driver();
}

static void links_3ds_after_fork(void)
{
}

static unsigned char *links_3ds_get_driver_param(void)
{
    return stracpy(cast_uchar "");
}

static void links_3ds_get_margin(int *left,
                                 int *right,
                                 int *top,
                                 int *bottom)
{
    if (left != NULL) {
        *left = 0;
    }
    if (right != NULL) {
        *right = 0;
    }
    if (top != NULL) {
        *top = 0;
    }
    if (bottom != NULL) {
        *bottom = 0;
    }
}

static int links_3ds_set_margin(int left,
                                int right,
                                int top,
                                int bottom)
{
    return left == 0 && right == 0 && top == 0 && bottom == 0 ? 0 : -1;
}

static int links_3ds_get_empty_bitmap(struct bitmap *dest)
{
    size_t allocation_size;

    if (dest == NULL || dest->x <= 0 || dest->y <= 0) {
        return -1;
    }

    if ((size_t)dest->x > SIZE_MAX / sizeof(uint16_t) / (size_t)dest->y) {
        return -1;
    }

    allocation_size = (size_t)dest->x * (size_t)dest->y * sizeof(uint16_t);
    dest->data = (unsigned char *)mem_alloc(allocation_size);
    memset(dest->data, 0, allocation_size);
    dest->skip = dest->x * (int)sizeof(uint16_t);
    dest->flags = NULL;
    return 0;
}

static void links_3ds_register_bitmap(struct bitmap *bmp)
{
    (void)bmp;
}

static void *links_3ds_prepare_strip(struct bitmap *bmp, int top, int lines)
{
    if (bmp == NULL || bmp->data == NULL || top < 0 || lines < 0 ||
        top > bmp->y || lines > bmp->y - top) {
        return NULL;
    }

    return bmp->data + (size_t)top * (size_t)bmp->skip;
}

static void links_3ds_commit_strip(struct bitmap *bmp, int top, int lines)
{
    (void)bmp;
    (void)top;
    (void)lines;
}

static void links_3ds_unregister_bitmap(struct bitmap *bmp)
{
    if (bmp == NULL) {
        return;
    }

    if (bmp->data != NULL) {
        mem_free(bmp->data);
        bmp->data = NULL;
    }

    bmp->skip = 0;
    bmp->flags = NULL;
}

static void links_3ds_draw_bitmap(struct graphics_device *dev,
                                  struct bitmap *bmp,
                                  int x,
                                  int y)
{
    struct links_3ds_device *private_data;
    uint16_t *destination;
    int destination_stride;
    int source_x = 0;
    int source_y = 0;
    int width;
    int height;
    int row;

    if (dev == NULL || bmp == NULL || bmp->data == NULL) {
        return;
    }

    private_data = links_3ds_private(dev);
    if (private_data == NULL || private_data->surface == NULL) {
        return;
    }

    width = bmp->x;
    height = bmp->y;

    if (x < dev->clip.x1) {
        source_x = dev->clip.x1 - x;
        width -= source_x;
        x = dev->clip.x1;
    }
    if (y < dev->clip.y1) {
        source_y = dev->clip.y1 - y;
        height -= source_y;
        y = dev->clip.y1;
    }
    if (x + width > dev->clip.x2) {
        width = dev->clip.x2 - x;
    }
    if (y + height > dev->clip.y2) {
        height = dev->clip.y2 - y;
    }
    if (width <= 0 || height <= 0) {
        return;
    }

    destination = gfx_surface_lock(private_data->surface);
    if (destination == NULL) {
        return;
    }

    destination_stride = gfx_surface_stride_bytes(private_data->surface);
    for (row = 0; row < height; ++row) {
        const unsigned char *source_row =
            bmp->data + (size_t)(source_y + row) * (size_t)bmp->skip +
            (size_t)source_x * sizeof(uint16_t);
        unsigned char *destination_row =
            (unsigned char *)destination +
            (size_t)(y + row) * (size_t)destination_stride +
            (size_t)x * sizeof(uint16_t);

        memcpy(destination_row,
               source_row,
               (size_t)width * sizeof(uint16_t));
    }

    gfx_surface_unlock(private_data->surface);
}

static long links_3ds_get_color(int rgb)
{
    uint8_t red = (uint8_t)(((unsigned int)rgb >> 16) & 0xffu);
    uint8_t green = (uint8_t)(((unsigned int)rgb >> 8) & 0xffu);
    uint8_t blue = (uint8_t)((unsigned int)rgb & 0xffu);

    return (long)gfx_rgb565(red, green, blue);
}

static void links_3ds_fill_area(struct graphics_device *dev,
                                int x1,
                                int y1,
                                int x2,
                                int y2,
                                long color)
{
    struct links_3ds_device *private_data = links_3ds_private(dev);

    if (private_data == NULL || private_data->surface == NULL) {
        return;
    }

    gfx_fill_rect(private_data->surface,
                  x1,
                  y1,
                  x2,
                  y2,
                  (uint16_t)color);
}

static void links_3ds_draw_hline(struct graphics_device *dev,
                                 int left,
                                 int y,
                                 int right,
                                 long color)
{
    struct links_3ds_device *private_data = links_3ds_private(dev);

    if (private_data == NULL || private_data->surface == NULL) {
        return;
    }

    gfx_draw_hline(private_data->surface,
                   left,
                   right,
                   y,
                   (uint16_t)color);
}

static void links_3ds_draw_vline(struct graphics_device *dev,
                                 int x,
                                 int top,
                                 int bottom,
                                 long color)
{
    struct links_3ds_device *private_data = links_3ds_private(dev);

    if (private_data == NULL || private_data->surface == NULL) {
        return;
    }

    gfx_draw_vline(private_data->surface,
                   top,
                   bottom,
                   x,
                   (uint16_t)color);
}

static int links_3ds_scroll(struct graphics_device *dev,
                            struct rect_set **set,
                            int scx,
                            int scy)
{
    (void)dev;
    (void)set;
    (void)scx;
    (void)scy;

    return 1;
}

static void links_3ds_set_clip_area(struct graphics_device *dev)
{
    struct links_3ds_device *private_data = links_3ds_private(dev);

    if (private_data == NULL || private_data->surface == NULL) {
        return;
    }

    gfx_set_clip(private_data->surface,
                 dev->clip.x1,
                 dev->clip.y1,
                 dev->clip.x2,
                 dev->clip.y2);
}

static void links_3ds_flush(struct graphics_device *dev)
{
    struct links_3ds_device *private_data = links_3ds_private(dev);

    if (private_data == NULL || private_data->surface == NULL) {
        return;
    }

    gfx_present(private_data->surface);
}

static int links_3ds_block(struct graphics_device *dev)
{
    (void)dev;
    return 0;
}

static int links_3ds_unblock(struct graphics_device *dev)
{
    (void)dev;
    return 0;
}

struct graphics_driver links_3ds_driver = {
    cast_uchar "3ds",
    links_3ds_init_driver,
    links_3ds_init_device,
    links_3ds_shutdown_device,
    links_3ds_shutdown_driver,
    links_3ds_emergency_shutdown,
    links_3ds_after_fork,
    links_3ds_get_driver_param,
    NULL,
    links_3ds_get_margin,
    links_3ds_set_margin,
    links_3ds_get_empty_bitmap,
    links_3ds_register_bitmap,
    links_3ds_prepare_strip,
    links_3ds_commit_strip,
    links_3ds_unregister_bitmap,
    links_3ds_draw_bitmap,
    links_3ds_get_color,
    links_3ds_fill_area,
    links_3ds_draw_hline,
    links_3ds_draw_vline,
    links_3ds_scroll,
    links_3ds_set_clip_area,
    links_3ds_flush,
    links_3ds_block,
    links_3ds_unblock,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    LINKS_3DS_DEPTH,
    LINKS_3DS_WIDTH,
    LINKS_3DS_HEIGHT,
    GD_ONLY_1_WINDOW | GD_NO_OS_SHELL | GD_NO_LIBEVENT,
    NULL
};
