#include <stdio.h>

int main(int argc, char **argv) {
    if (argc == 1)
        return 1;

    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
        return 1;

    printf("{ ");
    char c;
    while ((c = fgetc(f)) != EOF) {
        printf("0x%x, ", c);
    }
    printf("0x0 };\n");

    fclose(f);
    return 0;
}
