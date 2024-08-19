#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void print_active_window_title(Display* display) {
    Window active_window = get_active_window(display);
    if (active_window) {
        char* title = get_window_title(display, active_window);
        if (title) {
            printf("%s\n", title);
            XFree(title);
        } else {
            printf("\n"); // no title
        }
    } else {
        printf("\n"); // no title
    }
}

int main() {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    // Print the active window title on startup
    print_active_window_title(display);

    Window root = DefaultRootWindow(display);
    Atom active_window_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    XSelectInput(display, root, PropertyChangeMask);

    // Enter the event loop to listen for active window changes
    while (1) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == PropertyNotify && event.xproperty.atom == active_window_atom) {
            print_active_window_title(display);
        }
    }

    XCloseDisplay(display);
    return 0;
}
