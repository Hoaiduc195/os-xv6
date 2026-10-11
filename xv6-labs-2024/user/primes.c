#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int left) {
    int prime;
    if (read(left, &prime, sizeof(prime)) != sizeof(prime)) {
        close(left);
        exit(0);
    }
    printf("prime %d\n", prime);

    int right = -1;
    int n;
    while (read(left, &n, sizeof(n)) == sizeof(n)) {
        if (n % prime == 0) {
            continue;
        }
        if (right < 0) {
            int p[2];
            if (pipe(p) < 0) {
                fprintf(2, "primes: pipe failed\n");
                exit(1);
            }
            int pid = fork();
            if (pid < 0) {
                fprintf(2, "primes: fork failed\n");
                exit(1);
            }
            else if (pid == 0) {
                close(p[1]);
                close(left);
                primes(p[0]);
            }
            close(p[0]);
            right = p[1];
        }
        if (write(right, &n, sizeof(n)) != sizeof(n)) {
            fprintf(2, "primes: write failed\n");
            exit(1);
        }
    }
    close(left);
    if (right >= 0) {
        close(right);
        wait(0);
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "primes: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "primes: fork failed\n");
        exit(1);
    }
    else if (pid == 0) {
        close(p[1]);
        primes(p[0]);
    }
    else {
        close(p[0]);
        for (int i = 2; i <= 280; i++) {
            if (write(p[1], &i, sizeof(i)) != sizeof(i)) {
                fprintf(2, "primes: write failed\n");
                exit(1);
            }
        }
        close(p[1]);
        wait(0);
    }
    exit(0);
}