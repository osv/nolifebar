#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define BUFFER_SIZE 1024 * 1024
#define SPLIT_CHAR ':'

typedef struct Replacement {
    char *match_text;
    char *replace_text;
    struct Replacement *next;
} Replacement;

char *trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';

    return str;
}

Replacement *create_replacement(const char *match_text, const char *replace_text) {
    Replacement *new_node = (Replacement *)malloc(sizeof(Replacement));
    if (!new_node) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    new_node->match_text = strdup(match_text);
    new_node->replace_text = strdup(replace_text);
    new_node->next = NULL;
    return new_node;
}

void free_replacements(Replacement *head) {
    while (head) {
        Replacement *temp = head;
        head = head->next;
        free(temp->match_text);
        free(temp->replace_text);
        free(temp);
    }
}

int load_replacements(const char *filename, Replacement **head) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening replacements file: %s\n", filename);
        return -1;
    }

    Replacement *temp_head = NULL, *tail = NULL;
    size_t line_size = 0;
    char *line = NULL;

    while (getline(&line, &line_size, fp) != -1) {
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        if (line[0] == '\0') continue;

        char *colon = strchr(line, SPLIT_CHAR);
        if (!colon) {
            fprintf(stderr, "Invalid format in replacements file. Line: %s\n", line);
            continue;
        }

        *colon = '\0';
        char *match_text = trim(line);
        char *replace_text = trim(colon + 1);

        Replacement *new_node = create_replacement(match_text, replace_text);
        if (!new_node) {
            fclose(fp);
            free(line);
            free_replacements(temp_head);
            return -1;
        }

        if (!temp_head)
            temp_head = new_node;
        else
            tail->next = new_node;

        tail = new_node;
    }

    free(line);
    fclose(fp);

    *head = temp_head;
    return 0;
}

void process_line_inplace(char *buffer, size_t buffer_size, Replacement *replacements) {
    Replacement *current = replacements;

    while (current) {
        char *pos;
        size_t match_len = strlen(current->match_text);
        size_t replace_len = strlen(current->replace_text);

        while ((pos = strstr(buffer, current->match_text)) != NULL) {
            size_t remaining = strlen(pos + match_len);

            // If replacement text is longer than the match text, ensure buffer has enough space
            if (replace_len > match_len && strlen(buffer) + (replace_len - match_len) >= buffer_size) {
                fprintf(stderr, "Buffer too small for replacement\n");
                return;
            }

            // Shift buffer content if replacement text is not the same length as match text
            if (replace_len != match_len) {
                memmove(pos + replace_len, pos + match_len, remaining + 1);  // Include null terminator
            }

            // Copy replacement text into the position
            memcpy(pos, current->replace_text, replace_len);
        }

        current = current->next;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <replacement_file>\n"
               "\nPerforms in-place text replacements based on pairs specified in a watched file"
               "\n(reloading replacements if the file changes),"
               "\nand outputs the modified text to STDOUT"
               "\n", argv[0]);
        return 1;
    }

    const char *replacement_file = argv[1];
    Replacement *replacements = NULL;
    if (load_replacements(replacement_file, &replacements) != 0) {
        return 1;
    }

    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0) {
        perror("inotify_init1");
        free_replacements(replacements);
        return 1;
    }

    int wd = inotify_add_watch(inotify_fd, replacement_file, IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(inotify_fd);
        free_replacements(replacements);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    char last_line[BUFFER_SIZE];
    last_line[0] = '\0';  // Initialize last line as an empty string

    fd_set fds;
    int max_fd = (STDIN_FILENO > inotify_fd) ? STDIN_FILENO : inotify_fd;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(inotify_fd, &fds);

        int ret = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                if (feof(stdin)) {
                    break;
                } else {
                    perror("fgets");
                    continue;
                }
            }

            // Remove trailing newline
            buffer[strcspn(buffer, "\n")] = '\0';

            // Copy to last line buffer
            strncpy(last_line, buffer, BUFFER_SIZE);

            // Process line with replacements in place
            process_line_inplace(buffer, BUFFER_SIZE, replacements);
            printf("%s\n", buffer);
            fflush(stdout);
        }

        if (FD_ISSET(inotify_fd, &fds)) {
            char event_buf[EVENT_BUF_LEN];
            int length = read(inotify_fd, event_buf, EVENT_BUF_LEN);
            if (length < 0 && errno != EAGAIN) {
                perror("read");
                break;
            }

            int i = 0;
            while (i < length) {
                struct inotify_event *event = (struct inotify_event *)&event_buf[i];
                if (event->mask & IN_MODIFY) {
                    // File has been modified; reload replacements
                    free_replacements(replacements);
                    replacements = NULL;
                    if (load_replacements(replacement_file, &replacements) != 0) {
                        fprintf(stderr, "Error reading replacements file after modification\n");
                        break;
                    }

                    // Reprocess the last line
                    strncpy(buffer, last_line, BUFFER_SIZE);
                    process_line_inplace(buffer, BUFFER_SIZE, replacements);
                    printf("%s\n", buffer);
                    fflush(stdout);
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    free_replacements(replacements);
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);

    return 0;
}
