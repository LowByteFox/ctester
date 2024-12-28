#include "str.h"

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSZ 512

#include "program.h"
#include "err.h"

void handle_line(const char *str, const int len);
bool filter_space(const char a);
void usage();

int main(int argc, char **argv)
{
    setprogname(strrchr(argv[0], '/') + 1);

    if (argc == 1)
        usage();

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        err("%s", argv[1]);

    struct stat st;
    if (fstat(fd, &st) == -1)
        err(NULL);

    if (S_ISDIR(st.st_mode))
        die("%s is a directory!", argv[1]);

    bool parsing_tests = false;
    int n;
    int leftover = 0;
    char buffer[BUFSZ];

    while ((n = read(fd, buffer + leftover, BUFSZ - leftover)) > 0) {
        if (leftover > 0) {
            /* check for presence of \n */
            const char *check = strnchrend(buffer, '\n', BUFSZ);
            if (check == buffer + BUFSZ)
                die("The line is longer than %d bytes!\n", BUFSZ);

            n += leftover;
            leftover = 0;
        }

        const char *iter = buffer;

        /* go line by line */
        for (;n > 0;) {
            const char *next = strnchrend(iter, '\n', n);
            size_t len = next - iter;

            /* did we reach end of buffer? */
            if (iter + len == buffer + BUFSZ) {
                /* copy the unfinished line to the start */
                memcpy(buffer, iter, len);
                leftover = len;
                break;
            }

            /* work with the line here */
            handle_line(iter, len);

            iter += len + 1;
            n -= len + 1;
        }
    }

    close(fd);
    return 0;
}

void handle_line(const char *str, const int len)
{
    if (len == 0)
        return;

    int64_t new_len = len;

    if (isspace(*str)) {
        const char *filtered = strnfilter(str, filter_space, len);
        int64_t off = filtered - str;
        new_len = len - off;

        str += off;
    }

    printf("%.*s\n", (int) new_len, str);
}

bool filter_space(const char a)
{
    return !isspace(a);
}

void usage()
{
    fprintf(stderr, "usage: %s file\n", getprogname());
    exit(1);
}
