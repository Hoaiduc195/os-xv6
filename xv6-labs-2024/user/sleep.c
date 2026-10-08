#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: sleep ticks\n");
        exit(1);
    }
    char* s = argv[1];
    if (*s == '\0') {
        fprintf(2, "sleep: invalid ticks '%s'\n", argv[1]);
        exit(1);
    }
    for (; *s != '\0'; s++) {
        if (*s < '0' || *s > '9') {
            fprintf(2, "sleep: invalid ticks '%s'\n", argv[1]);
            exit(1);
        }
    }
    int ticks = atoi(argv[1]);
    sleep(ticks);
    exit(0);
}