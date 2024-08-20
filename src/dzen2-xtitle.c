#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the title of the currently active window
char* get_window_title(Display* display, Window window) {
    Atom prop = XInternAtom(display, "_NET_WM_NAME", False);
    Atom type;
    int format;
    unsigned long nitems, bytes_after;
    unsigned char* data = NULL;

    if (XGetWindowProperty(display, window, prop, 0, (~0L), False, AnyPropertyType,
                           &type, &format, &nitems, &bytes_after, &data) == Success) {
        if (data != NULL) {
            return (char*)data;
        }
    }

    return NULL;
}

// Function to get the currently active window
Window get_active_window(Display* display) {
    Atom prop = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    Atom type;
    int format;
    unsigned long nitems, bytes_after;
    unsigned char* data = NULL;
    Window active_window = 0;

    if (XGetWindowProperty(display, DefaultRootWindow(display), prop, 0, (~0L), False, AnyPropertyType,
                           &type, &format, &nitems, &bytes_after, &data) == Success) {
        if (data != NULL) {
            active_window = *((Window*)data);
            XFree(data);
        }
    }

    return active_window;
}

// Function to print the active window title or "No title"
void print_active_window_title(Display* display, Window window) {
    if (window) {
        char* title = get_window_title(display, window);
        if (title) {
            printf("%s\n", title);
            XFree(title);
        } else {
            printf("\n"); // no title
        }
    } else {
        printf("\n"); // no title
    }
    fflush(stdout);
}

// Function to listen for title changes of the active window only
void monitor_active_window(Display* display, Window root, Window *active_window) {
    Atom active_window_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    Atom net_wm_name_atom = XInternAtom(display, "_NET_WM_NAME", False);

    XSelectInput(display, root, PropertyChangeMask);

    while (1) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == PropertyNotify) {
            if (event.xproperty.window == root && event.xproperty.atom == active_window_atom) {
                // Active window changed
                Window new_active_window = get_active_window(display);
                if (new_active_window != *active_window) {
                    *active_window = new_active_window;
                    print_active_window_title(display, *active_window);

                    // Listen for title changes on the new active window
                    if (*active_window) {
                        XSelectInput(display, *active_window, PropertyChangeMask);
                    }
                }
            } else if (event.xproperty.window == *active_window && event.xproperty.atom == net_wm_name_atom) {
                // Title of the active window changed
                print_active_window_title(display, *active_window);
            }
        }
    }
}

int main() {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    Window root = DefaultRootWindow(display);
    Window active_window = get_active_window(display);

    // Print the active window title on startup
    print_active_window_title(display, active_window);

    // Listen for changes to the active window and its title
    monitor_active_window(display, root, &active_window);

    XCloseDisplay(display);
    return 0;
}
