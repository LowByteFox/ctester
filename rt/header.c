#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void ctester_main();

bool run_test(int (*test_fn)(), const char *name);

bool run_expect_fail(int (*test_fn)(), const char *name);

void print_summary(int pass, int fail, int xfail, int xpass);
