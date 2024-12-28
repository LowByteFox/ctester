#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "program.h"
#include "err.h"

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

    close(fd);
    return 0;
}

void usage()
{
    fprintf(stderr, "usage: %s file\n", getprogname());
    exit(1);
}
