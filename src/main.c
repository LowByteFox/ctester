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
#define strcheck(len, max, ptr, str) len >= max && strncmp(ptr, str, max)

#include "program.h"
#include "err.h"

struct test {
    struct test *next;
    char *name;
    int name_length;
    bool expect_fail;
};

struct context {
    int fd;
    bool parse_tests;
    bool generate_main;
    struct test *tests;
    struct test *last_test;
};

struct format {
    const char *test;
    const char *fail;
    const char *summary;
};

const char header[];
const char footer[];

const struct format fmts[] = {
    {
#include "../rt/fmts/basic.c"
    },
    {NULL},
};

void handle_line(const char *str, const int len, struct context *ctx);

void append_test(struct context *ctx, const char *str, const int len,
    const bool fail);

bool filter_start(const char a);
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

    struct context ctx = {0};
    ctx.fd = STDOUT_FILENO;
    ctx.generate_main = true;

    write(ctx.fd, header, strlen(header));

    /* write formats */
    int format = 0;
    const char *code = "const char *test_fmt = \"";
    write(ctx.fd, code, strlen(code));

    write(ctx.fd, fmts[format].test, strlen(fmts[format].test));

    code = "\";\n";
    write(ctx.fd, code, strlen(code));

    code = "const char *fail_fmt = \"";
    write(ctx.fd, code, strlen(code));

    write(ctx.fd, fmts[format].fail, strlen(fmts[format].fail));

    code = "\";\n\n";
    write(ctx.fd, code, strlen(code));

    code = "const char *summary_fmt = \"";
    write(ctx.fd, code, strlen(code));

    write(ctx.fd, fmts[format].summary, strlen(fmts[format].summary));

    code = "\";\n";
    write(ctx.fd, code, strlen(code));

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
            handle_line(iter, len, &ctx);

            iter += len + 1;
            n -= len + 1;
        }
    }

    /* generate testing function */

    code = "\nvoid ctester_main()\n"
           "{\n"
           "    int pass, fail, xpass, xfail, res;\n"
           "    pass = fail = xpass = xfail = 0;\n"
           "\n";
    write(ctx.fd, code, strlen(code));

    struct test *iter = ctx.tests;
    for (;iter != NULL; iter = iter->next) {
        if (iter->expect_fail == false)
            code = "    res = run_test(";
        else
            code = "    res = run_expect_fail(";

        write(ctx.fd, code, strlen(code));
        write(ctx.fd, iter->name, iter->name_length);
        write(ctx.fd, ", \"", 3);
        write(ctx.fd, iter->name, iter->name_length);
        write(ctx.fd, "\"", 1);

        if (iter->expect_fail == false)
            code = ");\n"
                   "    if (res == true)\n"
                   "        pass += 1;\n"
                   "    else\n"
                   "        fail += 1;\n\n";
        else
            code = ");\n"
                   "    if (res == true)\n"
                   "        xfail += 1;\n"
                   "    else\n"
                   "        xpass += 1;\n\n";
        write(ctx.fd, code, strlen(code));
    }

    code = "    print_summary(pass, fail, xfail, xpass);\n"
           "}\n";
    write(ctx.fd, code, strlen(code));

    /* generate main function unless requested not to */
    if (ctx.generate_main) {
        code = "int main()\n"
               "{\n"
               "    ctester_main();\n"
               "    return 0;\n"
               "}\n\n";

        write(ctx.fd, code, strlen(code));
    }

    write(ctx.fd, footer, strlen(footer));

    close(fd);
    return 0;
}

void handle_line(const char *str, const int len, struct context *ctx)
{
    if (len == 0) {
        write(ctx->fd, "\n", 1);
        return;
    }

    int64_t new_len = len;
    int64_t off = 0;

    if (isspace(*str)) {
        const char *filtered = strnfilter(str, filter_space, len);
        off = filtered - str;

        new_len -= off;
        str += off;
    }

    if (strcheck(new_len, 2, str, "%%") == 0) {
        ctx->parse_tests = !ctx->parse_tests;
        return;
    }

    if (ctx->parse_tests) {
        if (strcheck(new_len, 5, str, "%test") == 0) {
            append_test(ctx, str + 5, new_len - 5, false);
            return;
        } else if (strcheck(new_len, 5, str, "%fail") == 0) {
            append_test(ctx, str + 5, new_len - 5, true);
            return;
        } else if (strcheck(new_len, 6, str, "%bench") == 0) {
            /* parse bench */
            return;
        } else if (strcheck(new_len, 7, str, "%nomain") == 0) {
            ctx->generate_main = false;
            return;
        }
    }

    write(ctx->fd, str - off, len);
    write(ctx->fd, "\n", 1);
}

void append_test(struct context *ctx, const char *str, const int len,
    const bool fail)
{
    if (len <= 0)
        die("Expected test name!");

    if (!isspace(*str))
        die("Expected whitespace delimiter!");

    const char *filtered = strnfilter(str, filter_space, len);
    int64_t off = filtered - str;

    int64_t new_len = len - off;
    str += off;

    const char *end = strnfilter(str, filter_start, len);
    int64_t name_length = end - str;

    struct test *t = malloc(sizeof(*t));
    t->name_length = name_length;
    t->name = strndup(str, name_length);
    t->next = NULL;
    t->expect_fail = fail;

    write(ctx->fd, "int ", 4);
    write(ctx->fd, str, name_length);
    write(ctx->fd, "()", 2);

    if (end < str + new_len) {
        new_len -= name_length;
        str += name_length;

        write(ctx->fd, str, new_len);
    }
    write(ctx->fd, "\n", 1);

    if (ctx->tests == NULL) {
        ctx->tests = t;
        ctx->last_test = t;
    } else {
        ctx->last_test->next = t;
        ctx->last_test = t;
    }
}

bool filter_start(const char a)
{
    return isspace(a) || a == '{';
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

const char header[] =
#include "../rt/header.hex"

const char footer[] = 
#include "../rt/footer.hex"
