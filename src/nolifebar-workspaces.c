/* Prints desktops

   Example of output:

   1 - U 1 WWW
   2 - - 0 SYS
   3 A - 2 DOC
   4 - - 0 Music

   | | | | |
   | | | | +- desktop name
   | | | +- window count
   | | +-- urgent hint
   | +--- is active desktop or not
   +---- desktop num

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <stdbool.h>

// Function to check if a window has the "urgent" flag
bool get_wm_urgency(xcb_connection_t* c, xcb_window_t w) {
    xcb_icccm_wm_hints_t hints;
    if (xcb_icccm_get_wm_hints_reply(c, xcb_icccm_get_wm_hints(c, w), &hints, NULL)) {
        if (xcb_icccm_wm_hints_get_urgency(&hints) == XCB_ICCCM_WM_HINT_X_URGENCY)
            return true;
    }
    return false;
}

// Function to get the desktop index a window is on
unsigned int get_desktop_from_window(xcb_ewmh_connection_t *ewmh, xcb_window_t window) {
    unsigned int desktop = XCB_NONE;
    xcb_ewmh_get_wm_desktop_reply(ewmh, xcb_ewmh_get_wm_desktop(ewmh, window), &desktop, NULL);
    return desktop;
}

void print_workspaces(xcb_ewmh_connection_t *ewmh, xcb_window_t root) {
    xcb_ewmh_get_utf8_strings_reply_t names;
    uint32_t current_desktop;
    uint32_t num_desktops;
    xcb_ewmh_get_windows_reply_t window_list;

    uint32_t *window_count_per_desktop = NULL;
    uint8_t *desktop_has_urgent_window = NULL;

    // Get the number of desktops
    if (!xcb_ewmh_get_number_of_desktops_reply(ewmh, xcb_ewmh_get_number_of_desktops(ewmh, 0), &num_desktops, NULL)) return;

    // Get the current desktop
    if (!xcb_ewmh_get_current_desktop_reply(ewmh, xcb_ewmh_get_current_desktop(ewmh, 0), &current_desktop, NULL)) return;

    // Get the desktop names
    if (xcb_ewmh_get_desktop_names_reply(ewmh, xcb_ewmh_get_desktop_names(ewmh, 0), &names, NULL)) {
        // Get the list of windows
        if (xcb_ewmh_get_client_list_stacking_reply(ewmh, xcb_ewmh_get_client_list_stacking(ewmh, 0), &window_list, NULL)) {
            // Allocate arrays for window counts and urgency flags
            window_count_per_desktop = calloc(num_desktops, sizeof(uint32_t));
            desktop_has_urgent_window = calloc(num_desktops, sizeof(uint8_t));

            if (window_count_per_desktop && desktop_has_urgent_window) {
                // Iterate over windows and assign them to desktops
                for (uint32_t i = 0; i < window_list.windows_len; i++) {
                    uint32_t desktop_index = get_desktop_from_window(ewmh, window_list.windows[i]);
                    if (desktop_index < num_desktops) {
                        // Count of windows
                        window_count_per_desktop[desktop_index]++;

                        // Check if the window is urgent
                        if (get_wm_urgency(ewmh->connection, window_list.windows[i])) {
                            desktop_has_urgent_window[desktop_index]++;
                        }

                        // Add event mask to listen for property changes for each window
                        uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
                        xcb_change_window_attributes(ewmh->connection, window_list.windows[i], XCB_CW_EVENT_MASK, values);
                    }
                }

                // Print the desktop names, window counts, and urgency status
                char *name = names.strings;
                for (unsigned int i = 0, ws_index = 1; i < names.strings_len; i++) {
                    if (name[i] == '\0') {
                        printf("%i %s %s %u %s\n",
                               ws_index,
                               (ws_index == current_desktop) ? "A" : "-",
                               desktop_has_urgent_window[ws_index] ? "U" : "-",
                               window_count_per_desktop[ws_index],
                               name);
                        name += strlen(name) + 1;
                        ws_index++;
                        if (ws_index > num_desktops) break;
                    }
                }
            }

            if (window_count_per_desktop) free(window_count_per_desktop);
            if (desktop_has_urgent_window) free(desktop_has_urgent_window);
            xcb_ewmh_get_windows_reply_wipe(&window_list);
        }

        // Clean up desktop names
        xcb_ewmh_get_utf8_strings_reply_wipe(&names);
    }
    printf("\n");
}

// Handles property change events, checks if the urgency of a window has changed
void handle_property_notify(xcb_ewmh_connection_t *ewmh, xcb_property_notify_event_t *event) {
    // Reprint workspaces to reflect the change in urgency
    print_workspaces(ewmh, event->window);
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
            case XCB_PROPERTY_NOTIFY: {
                xcb_property_notify_event_t *property_event = (xcb_property_notify_event_t *)event;
                handle_property_notify(&ewmh, property_event);
                break;
            }
        }
        free(event);
    }

    xcb_ewmh_connection_wipe(&ewmh);
    xcb_disconnect(conn);
    return 0;
}
