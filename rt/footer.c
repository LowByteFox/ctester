bool run_test(int (*test_fn)(), const char *name)
{
    printf(test_fmt, name);
    pid_t pid = fork();

    if (pid < 0) {
        perror("Unable to create child process!");
        return false;
    } else if (pid == 0) {
        /* child process */
        exit(test_fn());
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0)
                return false;
        } else
            return false;
    }

    return true;
}

bool run_expect_fail(int (*test_fn)(), const char *name)
{
    printf(fail_fmt, name);
    pid_t pid = fork();

    if (pid < 0) {
        perror("Unable to create child process!");
        return true;
    } else if (pid == 0) {
        /* child process */
        exit(test_fn());
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0)
                return false;
        } else
            return true;
    }

    return true;
}

void print_summary(int pass, int fail, int xfail, int xpass)
{
    printf(summary_fmt, pass, fail, xpass, xfail);
}
