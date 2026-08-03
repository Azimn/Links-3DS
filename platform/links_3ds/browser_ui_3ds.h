#ifndef LINKS_3DS_BROWSER_UI_H
#define LINKS_3DS_BROWSER_UI_H

#include <stdbool.h>
#include <stddef.h>

#include "links_3ds_platform.h"

bool links_3ds_ui_init(void);
void links_3ds_ui_shutdown(void);
void links_3ds_ui_render(void);
void links_3ds_ui_set_status(const char *status);
void links_3ds_ui_set_url(const char *url);

/* Returns true when the touch was consumed by the lower-screen UI. */
bool links_3ds_ui_handle_touch(int x,
                               int y,
                               bool down,
                               bool held,
                               bool up,
                               const links_3ds_event_sink_t *sink);

/* Opens the system software keyboard and submits UTF-8 text to the sink. */
bool links_3ds_ui_open_keyboard(const links_3ds_event_sink_t *sink,
                                const char *initial_text,
                                const char *hint,
                                bool password);

#endif
