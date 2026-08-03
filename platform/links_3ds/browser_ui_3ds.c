#include "browser_ui_3ds.h"

#include <3ds.h>

#include <stdio.h>
#include <string.h>

#define UI_URL_HEIGHT 36
#define UI_TOOLBAR_TOP 192
#define UI_BUTTON_WIDTH 64
#define UI_TEXT_CAPACITY 512

static PrintConsole bottom_console;
static bool ui_ready;
static char current_url[UI_TEXT_CAPACITY] = "about:blank";
static char current_status[96] = "Ready";
static int last_touch_x = -1;
static int last_touch_y = -1;

static void emit_key(const links_3ds_event_sink_t *sink, links_3ds_key_t key)
{
    if (sink != NULL && sink->keyboard != NULL) {
        sink->keyboard(sink->context, key, 0u);
    }
}

static void render_button_row(void)
{
    printf("\x1b[24;1H[BACK] [FWD ] [LOAD] [HOME] [MENU]");
}

bool links_3ds_ui_init(void)
{
    if (ui_ready) {
        return true;
    }

    consoleInit(GFX_BOTTOM, &bottom_console);
    consoleSelect(&bottom_console);
    ui_ready = true;
    links_3ds_ui_render();
    return true;
}

void links_3ds_ui_shutdown(void)
{
    ui_ready = false;
    last_touch_x = -1;
    last_touch_y = -1;
}

void links_3ds_ui_set_status(const char *status)
{
    if (status == NULL) {
        status = "";
    }
    snprintf(current_status, sizeof(current_status), "%s", status);
}

void links_3ds_ui_set_url(const char *url)
{
    if (url == NULL || url[0] == '\0') {
        url = "about:blank";
    }
    snprintf(current_url, sizeof(current_url), "%s", url);
}

void links_3ds_ui_render(void)
{
    if (!ui_ready) {
        return;
    }

    consoleSelect(&bottom_console);
    printf("\x1b[2J\x1b[1;1H");
    printf("LINKS 3DS ALPHA\n");
    printf("URL: %.44s\n", current_url);
    printf("Status: %.37s\n", current_status);
    printf("----------------------------------------\n");
    printf("Touch here or press SELECT for URL input\n");
    printf("\n");
    printf("        TOUCH TRACKPAD AREA\n");
    printf("\n");
    printf("Drag: move pointer     Tap: click\n");
    printf("Circle Pad also moves the pointer\n");
    printf("L/R: page up/down      START: exit\n");
    render_button_row();
}

bool links_3ds_ui_open_keyboard(const links_3ds_event_sink_t *sink,
                                const char *initial_text,
                                const char *hint,
                                bool password)
{
    SwkbdState keyboard;
    SwkbdButton button;
    char text[UI_TEXT_CAPACITY];

    if (sink == NULL || sink->text == NULL) {
        return false;
    }

    memset(text, 0, sizeof(text));
    swkbdInit(&keyboard, SWKBD_TYPE_QWERTY, 2, UI_TEXT_CAPACITY - 1);
    swkbdSetButton(&keyboard, SWKBD_BUTTON_LEFT, "Cancel", false);
    swkbdSetButton(&keyboard, SWKBD_BUTTON_RIGHT, "Open", true);
    swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    if (hint != NULL) {
        swkbdSetHintText(&keyboard, hint);
    }
    if (initial_text != NULL) {
        swkbdSetInitialText(&keyboard, initial_text);
    }
    if (password) {
        swkbdSetPasswordMode(&keyboard, SWKBD_PASSWORD_HIDE_DELAY);
    }

    button = swkbdInputText(&keyboard, text, sizeof(text));
    links_3ds_ui_render();

    if (button != SWKBD_BUTTON_RIGHT || text[0] == '\0') {
        links_3ds_ui_set_status("Input cancelled");
        links_3ds_ui_render();
        return false;
    }

    links_3ds_ui_set_url(text);
    links_3ds_ui_set_status("Opening address");
    links_3ds_ui_render();
    sink->text(sink->context, text, strlen(text), true);
    return true;
}

static void handle_toolbar(int x, const links_3ds_event_sink_t *sink)
{
    int index = x / UI_BUTTON_WIDTH;

    switch (index) {
    case 0:
        emit_key(sink, LINKS_3DS_KEY_BACK);
        links_3ds_ui_set_status("Back");
        break;
    case 1:
        emit_key(sink, LINKS_3DS_KEY_FORWARD);
        links_3ds_ui_set_status("Forward");
        break;
    case 2:
        emit_key(sink, LINKS_3DS_KEY_RELOAD);
        links_3ds_ui_set_status("Reloading");
        break;
    case 3:
        emit_key(sink, LINKS_3DS_KEY_HOME);
        links_3ds_ui_set_status("Home");
        break;
    default:
        emit_key(sink, LINKS_3DS_KEY_ESCAPE);
        links_3ds_ui_set_status("Menu");
        break;
    }
    links_3ds_ui_render();
}

bool links_3ds_ui_handle_touch(int x,
                               int y,
                               bool down,
                               bool held,
                               bool up,
                               const links_3ds_event_sink_t *sink)
{
    int cursor_x;
    int cursor_y;

    if (!ui_ready || x < 0 || y < 0) {
        if (up) {
            last_touch_x = -1;
            last_touch_y = -1;
        }
        return false;
    }

    if (down && y < UI_URL_HEIGHT) {
        emit_key(sink, LINKS_3DS_KEY_GOTO_URL);
        links_3ds_ui_open_keyboard(sink, current_url,
                                   "Enter web address or search", false);
        return true;
    }

    if (down && y >= UI_TOOLBAR_TOP) {
        handle_toolbar(x, sink);
        return true;
    }

    if (y >= UI_URL_HEIGHT && y < UI_TOOLBAR_TOP) {
        gfx_get_cursor_pos(&cursor_x, &cursor_y);

        if (down) {
            last_touch_x = x;
            last_touch_y = y;
            if (sink != NULL && sink->pointer != NULL) {
                sink->pointer(sink->context, cursor_x, cursor_y,
                              LINKS_3DS_POINTER_DOWN, 1u);
            }
            return true;
        }

        if (held && last_touch_x >= 0 && last_touch_y >= 0) {
            cursor_x += x - last_touch_x;
            cursor_y += y - last_touch_y;
            if (cursor_x < 0) cursor_x = 0;
            if (cursor_y < 0) cursor_y = 0;
            if (cursor_x >= LINKS_3DS_SCREEN_WIDTH) cursor_x = LINKS_3DS_SCREEN_WIDTH - 1;
            if (cursor_y >= LINKS_3DS_SCREEN_HEIGHT) cursor_y = LINKS_3DS_SCREEN_HEIGHT - 1;
            gfx_set_cursor_pos(cursor_x, cursor_y);
            last_touch_x = x;
            last_touch_y = y;
            if (sink != NULL && sink->pointer != NULL) {
                sink->pointer(sink->context, cursor_x, cursor_y,
                              LINKS_3DS_POINTER_DRAG, 1u);
            }
            return true;
        }

        if (up) {
            if (sink != NULL && sink->pointer != NULL) {
                sink->pointer(sink->context, cursor_x, cursor_y,
                              LINKS_3DS_POINTER_UP, 1u);
            }
            last_touch_x = -1;
            last_touch_y = -1;
            return true;
        }
    }

    return false;
}
