#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <throttle_ms>\n", basename(argv[0]));
        fprintf(stderr, "Throttle STDIN.\n");
        return 1;
    }

    int throttle_ms = atoi(argv[1]);
    if (throttle_ms <= 0) {
        fprintf(stderr, "Throttle value must be a positive integer.\n");
        return 1;
    }

    struct timeval last_print_time;
    gettimeofday(&last_print_time, NULL);

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        long elapsed_ms = (current_time.tv_sec - last_print_time.tv_sec) * 1000 +
                          (current_time.tv_usec - last_print_time.tv_usec) / 1000;

        if (elapsed_ms >= throttle_ms) {
            printf("%s", buffer);
            fflush(stdout);
            last_print_time = current_time;
        }
    }

    return 0;
}
