#include "links.h"

#include "links_3ds_platform.h"

#include <stddef.h>

static struct graphics_device *bridge_device;

static int links_key_code(links_3ds_key_t key)
{
    switch (key) {
    case LINKS_3DS_KEY_UP:
        return KBD_UP;
    case LINKS_3DS_KEY_DOWN:
        return KBD_DOWN;
    case LINKS_3DS_KEY_LEFT:
        return KBD_LEFT;
    case LINKS_3DS_KEY_RIGHT:
        return KBD_RIGHT;
    case LINKS_3DS_KEY_ENTER:
        return KBD_ENTER;
    case LINKS_3DS_KEY_ESCAPE:
        return KBD_ESC;
    case LINKS_3DS_KEY_BACK:
        return KBD_BS;
    case LINKS_3DS_KEY_RELOAD:
        return 'r';
    case LINKS_3DS_KEY_PAGE_UP:
        return KBD_PAGE_UP;
    case LINKS_3DS_KEY_PAGE_DOWN:
        return KBD_PAGE_DOWN;
    case LINKS_3DS_KEY_CLOSE:
        return KBD_CLOSE;
    case LINKS_3DS_KEY_NONE:
    default:
        return 0;
    }
}

static void links_3ds_bridge_keyboard(void *context,
                                      links_3ds_key_t key,
                                      uint32_t modifiers)
{
    struct graphics_device *dev = (struct graphics_device *)context;
    int code;

    if (dev == NULL || dev != bridge_device || dev->keyboard_handler == NULL) {
        return;
    }

    code = links_key_code(key);
    if (code != 0) {
        dev->keyboard_handler(dev, code, (int)modifiers);
    }
}

static void links_3ds_bridge_pointer(void *context,
                                     int x,
                                     int y,
                                     links_3ds_pointer_action_t action,
                                     uint32_t buttons)
{
    struct graphics_device *dev = (struct graphics_device *)context;
    int event;

    if (dev == NULL || dev != bridge_device || dev->mouse_handler == NULL) {
        return;
    }

    switch (action) {
    case LINKS_3DS_POINTER_DOWN:
        event = B_DOWN;
        break;
    case LINKS_3DS_POINTER_UP:
        event = B_UP;
        break;
    case LINKS_3DS_POINTER_DRAG:
        event = B_DRAG;
        break;
    case LINKS_3DS_POINTER_MOVE:
    default:
        event = B_MOVE;
        break;
    }

    if ((buttons & 1u) != 0u || action == LINKS_3DS_POINTER_UP) {
        event |= B_LEFT;
    }

    dev->mouse_handler(dev, x, y, event);
}

void links_3ds_event_bridge_attach(struct graphics_device *dev)
{
    links_3ds_event_sink_t sink;

    bridge_device = dev;

    if (dev == NULL) {
        links_3ds_platform_set_event_sink(NULL);
        return;
    }

    sink.context = dev;
    sink.keyboard = links_3ds_bridge_keyboard;
    sink.pointer = links_3ds_bridge_pointer;
    links_3ds_platform_set_event_sink(&sink);
}

void links_3ds_event_bridge_detach(struct graphics_device *dev)
{
    if (dev != NULL && dev != bridge_device) {
        return;
    }

    links_3ds_platform_set_event_sink(NULL);
    bridge_device = NULL;
}
