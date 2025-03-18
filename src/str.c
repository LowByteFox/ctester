#include "str.h"

#include <assert.h>

const char *strnchrend(const char *str, char c, int n)
{
    assert(str != (void*) 0);
    assert(n >= 0);

    for (; *str != '\0' && n > 0; str++, n--)
        if (*str == c)
            return str;

    return str;
}

const char *strnfilter(const char *str, bool (*fn)(const char a), int n)
{
    assert(str != (void*) 0);
    assert(fn != (void*) 0);

    for (; *str != '\0' && n > 0; str++, n--)
        if (fn(*str))
            return str;

    return (void*) 0;
}
