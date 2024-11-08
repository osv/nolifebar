#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

void print_current_layout_and_locks(Display *dpy) {
    XkbStateRec state;
    XkbGetState(dpy, XkbUseCoreKbd, &state);

    XkbDescRec *kbdDesc = XkbAllocKeyboard();
    if (kbdDesc == NULL) {
        fprintf(stderr, "Failed to allocate keyboard description.\n");
        return;
    }

    kbdDesc->dpy = dpy;
    if (XkbGetNames(dpy, XkbGroupNamesMask, kbdDesc) != Success) {
        fprintf(stderr, "Failed to get keyboard names.\n");
        XkbFreeKeyboard(kbdDesc, 0, True);
        return;
    }

    char *group_name = XGetAtomName(dpy, kbdDesc->names->groups[state.group]);

    // Check Caps Lock and Num Lock state
    unsigned int n;
    XkbGetIndicatorState(dpy, XkbUseCoreKbd, &n);
    int caps_lock = n & 0x01;
    int num_lock = (n >> 1) & 0x01;

    printf("Layout: %s\n"                "Caps Lock: %s\n"         "Num Lock: %s\n\n",
           group_name ? group_name : "?", caps_lock ? "ON" : "OFF", num_lock ? "ON" : "OFF");

    fflush(stdout);
    XkbFreeKeyboard(kbdDesc, 0, True);
}

int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fprintf(stderr, "Unable to open display\n");
        exit(1);
    }

    int xkbOpcode, xkbEvent, xkbError, xkbMajorVersion = XkbMajorVersion, xkbMinorVersion = XkbMinorVersion;
    if (!XkbQueryExtension(dpy, &xkbOpcode, &xkbEvent, &xkbError, &xkbMajorVersion, &xkbMinorVersion)) {
        fprintf(stderr, "XKB extension not available\n");
        XCloseDisplay(dpy);
        exit(1);
    }

    XkbSelectEventDetails(dpy, XkbUseCoreKbd, XkbStateNotify, XkbAllStateComponentsMask, XkbGroupStateMask);
    XkbSelectEventDetails(dpy, XkbUseCoreKbd, XkbIndicatorStateNotify, XkbAllIndicatorsMask, XkbAllIndicatorsMask);

    print_current_layout_and_locks(dpy);

    while (1) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == xkbEvent) {
            XkbEvent *xkbEv = (XkbEvent *)&ev;
            if (xkbEv->any.xkb_type == XkbStateNotify || xkbEv->any.xkb_type == XkbIndicatorStateNotify) {
                print_current_layout_and_locks(dpy);
            }
        }
    }

    XCloseDisplay(dpy);
    return 0;
}
