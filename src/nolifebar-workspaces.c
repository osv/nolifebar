#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

void print_workspaces(xcb_ewmh_connection_t *ewmh, xcb_window_t root) {
    xcb_ewmh_get_utf8_strings_reply_t names;
    xcb_ewmh_get_windows_reply_t windows;
    uint32_t current_desktop;

    if (xcb_ewmh_get_desktop_names_reply(ewmh, xcb_ewmh_get_desktop_names(ewmh, 0), &names, NULL)) {
        if (xcb_ewmh_get_current_desktop_reply(ewmh, xcb_ewmh_get_current_desktop(ewmh, 0), &current_desktop, NULL)) {
            if (xcb_ewmh_get_client_list_stacking_reply(ewmh, xcb_ewmh_get_client_list_stacking(ewmh, 0), &windows, NULL)) {
                char *name = names.strings;
                for (unsigned int i = 0, ws_index = 0; i < names.strings_len; i++) {
                    if (name[i] == '\0') {
                        printf("%s%s(%d)\t",
                               (ws_index == current_desktop) ? "[active] " : "",
                               name,
                               windows.windows_len);
                        name += strlen(name) + 1;
                        ws_index++;
                    }
                }
                printf("\n");
                xcb_ewmh_get_windows_reply_wipe(&windows);
            }
        }
        xcb_ewmh_get_utf8_strings_reply_wipe(&names);
    }
}

void handle_property_notify(xcb_ewmh_connection_t *ewmh, xcb_window_t root) {
    print_workspaces(ewmh, root);
}

int main() {
    xcb_connection_t *conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(conn)) {
        fprintf(stderr, "Error: Can't connect to X server\n");
        return -1;
    }

    xcb_ewmh_connection_t ewmh;
    xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(conn, &ewmh);
    xcb_ewmh_init_atoms_replies(&ewmh, cookie, NULL);

    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;

    xcb_window_t root = screen->root;

    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(conn, root, XCB_CW_EVENT_MASK, values);

    print_workspaces(&ewmh, root);

    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(conn))) {
        switch (event->response_type & ~0x80) {
            case XCB_PROPERTY_NOTIFY:
                handle_property_notify(&ewmh, root);
                break;
        }
        free(event);
    }

    xcb_ewmh_connection_wipe(&ewmh);
    xcb_disconnect(conn);
    return 0;
}
