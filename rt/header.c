#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void ctester_main();

/* returns true if test succeeded, false otherwise */
bool run_test(int (*test_fn)(), const char *name);

/* returns true if test failed, true otherwise */
bool run_expect_fail(int (*test_fn)(), const char *name);

/* prints summary of tests */
void print_summary(int pass, int fail, int xfail, int xpass);
