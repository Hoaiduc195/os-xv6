#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }
    int q[2];
    if (pipe(q) < 0) {
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "pingpong: fork failed\n");
        exit(1);
    }
    else if (pid > 0) {
        close(p[0]);
        close(q[1]);
        char c = 'x';
        if (write(p[1], &c, 1) != 1) {
            fprintf(2, "pingpong: write failed\n");
            exit(1);
        }
        close(p[1]);
        wait(0);
        if (read(q[0], &c, 1) != 1) {
            fprintf(2, "pingpong: read failed\n");
            exit(1);
        }
        printf("%d: received pong\n", getpid());
        close(q[0]);
    }
    else {
        close(p[1]);
        close(q[0]);
        char c;
        if (read(p[0], &c, 1) != 1) {
            fprintf(2, "pingpong: read failed\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());
        if (write(q[1], &c, 1) != 1) {
            fprintf(2, "pingpong: write failed\n");
            exit(1);
        }
        close(p[0]);
        close(q[1]);
    }
    exit(0);
}