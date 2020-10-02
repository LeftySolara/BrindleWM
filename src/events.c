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

#include "events.h"
#include "logger.h"

#include <stdlib.h>

void handle_expose(xcb_generic_event_t *event)
{
    xcb_expose_event_t *ev = (xcb_expose_event_t *)event;
    log_debug("Window %d exposed.\n", ev->window);   
}

void handle_button_press(xcb_generic_event_t *event)
{
    xcb_button_press_event_t *ev = (xcb_button_press_event_t *)event;
    log_debug("Button pressed in window %d.\n", ev->event);
}

void handle_key_press(xcb_generic_event_t *event)
{
    xcb_key_press_event_t *ev = (xcb_key_press_event_t *)event;
    log_debug("Key %d pressed.\n", ev->detail);
}

void handle_create_notify(xcb_generic_event_t *event)
{
    xcb_create_notify_event_t *ev = (xcb_create_notify_event_t *)event;
    log_debug("Window %d created with parent %d.\n", ev->window, ev->parent);
}

void handle_map_request(xcb_connection_t *connection, xcb_screen_t *screen, xcb_generic_event_t *event)
{
    xcb_map_request_event_t *ev = (xcb_map_request_event_t *)event;
    log_debug("Map request received for window %d.\n", ev->window);

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
}

void handle_unmap_notify(xcb_connection_t *connection, xcb_generic_event_t *event)
{
    xcb_unmap_notify_event_t *ev = (xcb_unmap_notify_event_t *)event;
    log_debug("Unmap request received from window %d.\n", ev->window);

    xcb_unmap_window(connection, ev->event);
    xcb_flush(connection);
}

void handle_destroy_notify()
{
    log_debug("Destroy request received.\n");
}

void handle_events(xcb_connection_t *connection, xcb_screen_t *screen)
{
    xcb_generic_event_t *event;
    int done = 0;

    while (!done && (event = xcb_wait_for_event(connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE:
            handle_expose(event);
            break;
        case XCB_BUTTON_PRESS:
            handle_button_press(event);
            break;
        case XCB_KEY_PRESS:
            handle_key_press(event);
            break;
        case XCB_CREATE_NOTIFY:
            handle_create_notify(event);
            break;
        case XCB_MAP_REQUEST:
            handle_map_request(connection, screen, event);
            break;
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify(connection, event);
            break;
        case XCB_DESTROY_NOTIFY:
            handle_destroy_notify();
            break;
        default:
            break;
        }
        free(event);
    }
}
