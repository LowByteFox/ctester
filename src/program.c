#include "program.h"

static const char *p;

void setprogname(const char *prog)
{
    p = prog;
}

const char *getprogname()
{
    return p;
}
