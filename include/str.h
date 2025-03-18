#ifndef _STR_H_
#define _STR_H_

#include <stdbool.h>

const char *strnchrend(const char *str, char c, int n);
const char *strnfilter(const char *str, bool (*fn)(const char a), int n);

#endif
