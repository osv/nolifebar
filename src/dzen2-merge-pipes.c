#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

// Enable or disable debug prints
// #define DEBUG_EVENT 1

#ifdef DEBUG_EVENT
#define DEBUG_PRINT(...) if (DEBUG_EVENT) { printf("DBG: " __VA_ARGS__); }
#else
#define DEBUG_PRINT(...)
#endif

char **last_lines; // Array to store the last line from each monitored file or FIFO
char **file_names; // Array to store the file names without paths

void chomp(char *line) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

void update_last_line(int index, char *new_line) {
    chomp(new_line);
    if (last_lines[index]) {
        free(last_lines[index]);
    }
    last_lines[index] = strdup(new_line);
}

void process_template(const char *template, char **placeholders) {
    char result[4096] = {0};
    char *output = result;
    while (*template) {
        if (*template == '{') {
            template++;
            char key[256];
            int key_len = 0;
            while (*template && *template != '}') {
                key[key_len++] = *template++;
            }
            key[key_len] = '\0';
            if (*template == '}') template++;

            for (int i = 0; placeholders[i]; i++) {
                if (strcmp(key, file_names[i]) == 0 && last_lines[i]) {
                    output += sprintf(output, "%s", last_lines[i]);
                    break;
                }
            }
        } else {
            *output++ = *template++;
        }
    }
    *output = '\0';
    printf("%s\n", result);
    fflush(stdout);
}

void update_last_line_of_file(const char *filename, int index) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        update_last_line(index, line);
    }

    if (line) {
        free(line);
    }

    fclose(file);
}

void update_last_line_from_fifo(int *fd, int index, const char *filename) {
    char buffer[8 * 1024];
    ssize_t num_bytes;

    while ((num_bytes = read(*fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[num_bytes] = '\0';  // Ensure null termination for string operations

        char *rest = buffer;
        char *token;
        while ((token = strtok_r(rest, "\n", &rest))) {
            update_last_line(index, token);
        }
    }

    if (num_bytes == 0) {
        // Close and reopen the FIFO to reset its state.
        close(*fd);
        *fd = open(filename, O_RDONLY | O_NONBLOCK);
        if (*fd == -1) {
            perror("reopen FIFO");
            exit(EXIT_FAILURE);
        }
        DEBUG_PRINT("FIFO reopened\n");
    } else if (num_bytes < 0 && errno != EAGAIN) {
        perror("read");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <template> <file1> <file2> ... <fileN>\n", basename(argv[0]));
        fprintf(stderr, "This program monitors specified files and FIFOs, \n\
reads the last line from each, and replaces placeholders\n\
in a given template with these last lines.\n\n");
        fprintf(stderr, "Usage: \n\
  touch /tmp/template /tmp/file1 /tmp/file2\n\
\n\
  # Run the program \n\
  %s /tmp/template /tmp/file1 /tmp/file2\n\
  # Write template\n\
  echo \"{file1}::{file2}\" > /tmp/template\n\
\n\
  echo \"Content of file1\" > /tmp/file1\n\
  echo \"Content of file2\" > /tmp/file2\n\
  # Output: \n\
  # \"Content of file1::Content of file2\"\n", basename(argv[0]));

        exit(EXIT_FAILURE);
    }

    int length, i = 0;
    int fd;
    int *wd = calloc(argc - 1, sizeof(int));
    int *fifo_fds = calloc(argc - 1, sizeof(int));
    last_lines = calloc(argc, sizeof(char *));
    file_names = calloc(argc, sizeof(char *));
    char buffer[EVENT_BUF_LEN];
    fd_set read_fds;
    int max_fd = 0;

    if (!wd || !fifo_fds || !last_lines || !file_names) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }
    DEBUG_PRINT("inotify_init() succeeded, fd: %d\n", fd);
    max_fd = fd;

    for (i = 1; i < argc; i++) {
        file_names[i - 1] = strdup(basename(argv[i]));
        if (!file_names[i - 1]) {
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }

        struct stat st;
        if (stat(argv[i], &st) == 0 && S_ISFIFO(st.st_mode)) {
            fifo_fds[i - 1] = open(argv[i], O_RDONLY | O_NONBLOCK);
            if (fifo_fds[i - 1] < 0) {
                perror("open FIFO");
                exit(EXIT_FAILURE);
            }
            DEBUG_PRINT("Opened FIFO '%s' (fd: %d)\n", argv[i], fifo_fds[i - 1]);
            if (fifo_fds[i - 1] > max_fd) {
                max_fd = fifo_fds[i - 1];
            }
        } else {
            wd[i - 1] = inotify_add_watch(fd, argv[i], IN_MODIFY);
            if (wd[i - 1] == -1) {
                fprintf(stderr, "Cannot watch '%s': %s\n", argv[i], strerror(errno));
                exit(EXIT_FAILURE);
            } else {
                DEBUG_PRINT("Watching '%s' (wd: %d)\n", argv[i], wd[i - 1]);
            }
        }
    }

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        for (i = 0; i < argc - 1; i++) {
            if (fifo_fds[i] != -1) {
                FD_SET(fifo_fds[i], &read_fds);
            }
        }

        DEBUG_PRINT("Waiting for events...\n");
        int ret = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(fd, &read_fds)) {
            length = read(fd, buffer, EVENT_BUF_LEN);
            if (length < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            DEBUG_PRINT("Read %d bytes from inotify fd\n", length);

            i = 0;
            while (i < length) {
                struct inotify_event *event = (struct inotify_event *)&buffer[i];
                DEBUG_PRINT("Event: wd=%d mask=%u cookie=%u len=%u\n", event->wd, event->mask, event->cookie, event->len);
                if (event->len) {
                    DEBUG_PRINT("Event name: %s\n", event->name);
                    if (event->mask & IN_MODIFY) {
                        DEBUG_PRINT("File '%s' was modified\n", event->name);
                        update_last_line_of_file(event->name, i);
                    }
                } else {
                    DEBUG_PRINT("Event has no associated filename (event->len is 0).\n");
                    for (int j = 1; j < argc; j++) {
                        if (wd[j - 1] == event->wd) {
                            DEBUG_PRINT("File descriptor %d corresponds to file '%s'\n", event->wd, argv[j]);
                            update_last_line_of_file(argv[j], j-1);
                            break;
                        }
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }

        for (i = 0; i < argc - 1; i++) {
            if (fifo_fds[i] != -1 && FD_ISSET(fifo_fds[i], &read_fds)) {
                DEBUG_PRINT("FIFO '%s' is readable\n", argv[i + 1]);
                update_last_line_from_fifo(&fifo_fds[i], i, argv[i + 1]);
            }
        }

        if (last_lines[0] != NULL) {
            // Process the template if the first file/fifo has data
            process_template(last_lines[0], file_names);
        }
    }

    // Clean up resources
    for (i = 0; i < argc - 1; i++) {
        if (wd[i] != -1) {
            inotify_rm_watch(fd, wd[i]);
        }
        if (fifo_fds[i] != -1) {
            close(fifo_fds[i]);
        }
        if (file_names[i]) {
            free(file_names[i]);
        }
        if (last_lines[i]) {
            free(last_lines[i]);
        }
    }

    close(fd);
    free(wd);
    free(fifo_fds);
    free(last_lines);
    free(file_names);
    return 0;
}
