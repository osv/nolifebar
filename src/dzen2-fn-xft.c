#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <libgen.h>

#define BUFFER_SIZE 8 * 1024

int can_draw_char(XftFont *font, FcChar32 c) {
    return XftCharExists(NULL, font, c);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <font1> <font2> ... <fontN>\n", basename(argv[0]));
        fprintf(stderr, "Read STDIN and select first possible XFT font for rendering each chars and print dzen2 control sequence: ^fn()");
        return 1;
    }

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    // Load the fonts
    XftFont **fonts = malloc((argc - 1) * sizeof(XftFont *));
    if (!fonts) {
        fprintf(stderr, "Failed to allocate memory for fonts\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        fonts[i - 1] = XftFontOpenName(dpy, DefaultScreen(dpy), argv[i]);
        if (!fonts[i - 1]) {
            fprintf(stderr, "Failed to load font: %s\n", argv[i]);
            return 1;
        }
    }

    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        int current_font_index = -1;
        char *ptr = buffer;

        while (*ptr) {
            // Decode UTF-8 character
            int len;
            FcChar32 c;
            if ((unsigned char)ptr[0] < 0x80) {
                len = 1;
                c = ptr[0];
            } else if ((unsigned char)ptr[0] < 0xE0) {
                len = 2;
                c = ((ptr[0] & 0x1F) << 6) | (ptr[1] & 0x3F);
            } else if ((unsigned char)ptr[0] < 0xF0) {
                len = 3;
                c = ((ptr[0] & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
            } else {
                len = 4;
                c = ((ptr[0] & 0x07) << 18) | ((ptr[1] & 0x3F) << 12) | ((ptr[2] & 0x3F) << 6) | (ptr[3] & 0x3F);
            }

            int found_font = 0;
            for (int i = 0; i < argc - 1; i++) {
                if (can_draw_char(fonts[i], c)) {
                    if (i != current_font_index) {
                        fprintf(stdout, "^fn(%s)", argv[i + 1]);
                        current_font_index = i;
                    }
                    found_font = 1;
                    break;
                }
            }

            if (*ptr != '\n' && !found_font) {
                fprintf(stderr, "No suitable font found for character: U+%04X\n", c);
            } else {
                for (int i = 0; i < len; i++) {
                    putchar(ptr[i]);
                }
            }

            ptr += len;
        }

        fflush(stdout);
    }

    for (int i = 0; i < argc - 1; i++) {
        XftFontClose(dpy, fonts[i]);
    }
    free(fonts);
    XCloseDisplay(dpy);

    return 0;
}
