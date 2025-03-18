#include "err.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

void warn(const char *fmt, ...)
{
    fprintf(stderr, "%s: ", getprogname());

    if (fmt == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", strerror(errno));
}

void err(const char *fmt, ...)
{
    fprintf(stderr, "%s: ", getprogname());
    if (fmt == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(errno);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, ": %s\n", strerror(errno));
    exit(errno);
}

void die(const char *fmt, ...)
{
    fprintf(stderr, "%s: ", getprogname());

    if (fmt == NULL)
        exit(errno);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);
#ifdef NDEBUG
    exit(1);
#else
    abort();
#endif
}
