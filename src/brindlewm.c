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

#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

int main()
{
    int default_screen;
    xcb_connection_t *connection;
    xcb_screen_t *screen;

    connection = xcb_connect(NULL, &default_screen);
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    if (xcb_connection_has_error(connection)) {
        fprintf(stderr, "ERROR: could not connect to X server.\n");
        exit(EXIT_FAILURE);
    }
    else {
        fprintf(stdout, "Successfully connected to X server.\n");
    }

    /* Define the application as a window manager. */
    const uint32_t root_mask_val[] = {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_ENTER_WINDOW          | XCB_EVENT_MASK_LEAVE_WINDOW        |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY      | XCB_EVENT_MASK_PROPERTY_CHANGE     |
        XCB_EVENT_MASK_BUTTON_PRESS          | XCB_EVENT_MASK_BUTTON_RELEASE      |
        XCB_EVENT_MASK_FOCUS_CHANGE          | XCB_EVENT_MASK_BUTTON_MOTION       |
        XCB_EVENT_MASK_KEY_PRESS             | XCB_EVENT_MASK_KEY_RELEASE
    };

    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(
        connection, screen->root, XCB_CW_EVENT_MASK, root_mask_val);
    xcb_generic_error_t *error = xcb_request_check(connection, cookie);

    if (error) {
        xcb_disconnect(connection);
        fprintf(stderr, "ERROR: Another window manager is already running (X error %d). Exiting...\n",
                error->error_code);
        exit(EXIT_FAILURE);
    }

    xcb_flush(connection);

    /* Main event loop */
    xcb_generic_event_t *event;
    int done = 0;
    while (!done && (event = xcb_wait_for_event(connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            xcb_expose_event_t *ev = (xcb_expose_event_t *)event;
            fprintf(stdout, "Window %d exposed.\n", ev->window);
            break;
        }
        case XCB_BUTTON_PRESS: {
            xcb_button_press_event_t *ev = (xcb_button_press_event_t *)event;
            fprintf(stdout, "Button pressed in window %d.\n", ev->event);
            break;
        }
        case XCB_KEY_PRESS: {
            xcb_key_press_event_t *ev = (xcb_key_press_event_t *)event;
            fprintf(stdout, "Key %d pressed.\n", ev->detail);
            if (ev->detail == 37) {
                done = 1;
            }
            break;
        }
        case XCB_CREATE_NOTIFY: {
            xcb_create_notify_event_t *ev = (xcb_create_notify_event_t *)event;
            fprintf(stdout, "Window %d created with parent %d.\n", ev->window, ev->parent);
            break;
        }
        case XCB_MAP_REQUEST: {
            xcb_map_request_event_t *ev = (xcb_map_request_event_t *)event;
            fprintf(stdout, "Map request received for window %d.\n", ev->window);

            xcb_window_t frame = xcb_generate_id(connection);
            uint32_t frame_mask = XCB_CW_EVENT_MASK;
            const uint32_t frame_mask_values[] = {
                XCB_EVENT_MASK_BUTTON_PRESS |           /* Mouse button is pressed. */
                XCB_EVENT_MASK_BUTTON_RELEASE |         /* Mouse button is released. */
                XCB_EVENT_MASK_POINTER_MOTION |         /* Mouse is moved. */
                XCB_EVENT_MASK_EXPOSURE |               /* The window needs to be redrawn. */
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |       /* The frame window gets destroyed. */
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |  /* The application tries to resize itself. */
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |    /* Subwindows get notifications. */
                XCB_EVENT_MASK_ENTER_WINDOW             /* User moves cursor inside window. */
            };

            xcb_window_t child = ev->window;
            uint32_t child_mask = XCB_CW_EVENT_MASK;
            const uint32_t child_mask_values[] = {
                XCB_EVENT_MASK_PROPERTY_CHANGE |    /* A window property changes. */
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |   /* The window gets destroyed. */
                XCB_EVENT_MASK_FOCUS_CHANGE         /* The window gains/loses focus. */
            };

            xcb_window_t to_be_mapped = ev->window;
            xcb_get_geometry_reply_t *tbm_window_geometry = xcb_get_geometry_reply(
                connection, xcb_get_geometry(connection, to_be_mapped), NULL);

            xcb_create_window(connection,
                              XCB_COPY_FROM_PARENT,
                              frame, 
                              screen->root,
                              tbm_window_geometry->x,
                              tbm_window_geometry->y,
                              tbm_window_geometry->width,
                              tbm_window_geometry->height,
                              1,
                              XCB_WINDOW_CLASS_INPUT_OUTPUT,
                              screen->root_visual,
                              frame_mask,
                              frame_mask_values);

            xcb_reparent_window(connection, to_be_mapped, frame, 0, 0);
            xcb_change_window_attributes(connection, child, child_mask, child_mask_values);

            xcb_map_window(connection, frame);
            xcb_map_window(connection, to_be_mapped);

            xcb_flush(connection);
            break;
        }
        case XCB_UNMAP_NOTIFY: {
            xcb_unmap_notify_event_t *ev = (xcb_unmap_notify_event_t *)event;
            fprintf(stdout, "Unmap request received from window %d.\n", ev->window);

            xcb_unmap_window(connection, ev->event);
            xcb_flush(connection);
            break;
        }
        case XCB_DESTROY_NOTIFY: {
            fprintf(stdout, "Destroy request received.\n");
            break;
        }
        default:
            break;
        }
        free(event);
    }

    xcb_disconnect(connection);

    return 0;
}
