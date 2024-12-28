#include "str.h"

#include <assert.h>

const char *strnchrnul(const char *str, char c, int n)
{
    assert(str != (void*) 0);
    assert(n >= 0);

    for (; *str != '\0' && n > 0; str++, n--)
        if (*str == c)
            return str;

    return str;
}
