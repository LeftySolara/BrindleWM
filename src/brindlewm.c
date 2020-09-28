/* Copyright (c) 2020, Jalen Adams
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

int main()
{
    int default_screen;
    xcb_connection_t *connection = xcb_connect(NULL, &default_screen);
    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    if (xcb_connection_has_error(connection)) {
        log_error("Error connecting to X server.");
        exit(1);
    }
    
    /* Define the application as a window manager. */
    const uint32_t event_mask_val[] = {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_ENTER_WINDOW          | XCB_EVENT_MASK_LEAVE_WINDOW        |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY      | XCB_EVENT_MASK_PROPERTY_CHANGE     |
        XCB_EVENT_MASK_BUTTON_PRESS          | XCB_EVENT_MASK_BUTTON_RELEASE      |
        XCB_EVENT_MASK_FOCUS_CHANGE
    };
    xcb_change_window_attributes(connection, screen->root,
                                 XCB_CW_EVENT_MASK, event_mask_val);

    xcb_aux_sync(connection);
    if (xcb_poll_for_event(connection) != NULL) {
        log_error("Another window manager is already running.");
        exit(1);
    }
    xcb_flush(connection);

    /* Main event loop */
    int done = 0;
    xcb_generic_event_t *event;
    while (!done && (event = xcb_wait_for_event(connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_BUTTON_PRESS: {
            xcb_button_press_event_t *press_event = (xcb_button_press_event_t *)event;
            if (press_event->detail == XCB_BUTTON_INDEX_1) {
                log_info("Left click");
                break;
            }
            else if (press_event->detail == XCB_BUTTON_INDEX_3) {
                log_info("Right click");
                break;
            }
            break;
        }
        default: {
            break;
        }
        }
    }

    xcb_disconnect(connection);

    return 0;
}
