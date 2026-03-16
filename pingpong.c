#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define ITERATIONS 100000

double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

int main() {
    int parent_to_child[2];
    int child_to_parent[2];
    pid_t pid;
    char byte = 'A';

    if (pipe(parent_to_child) == -1) {
        perror("pipe parent_to_child");
        return 1;
    }

    if (pipe(child_to_parent) == -1) {
        perror("pipe child_to_parent");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process
        close(parent_to_child[1]); // child reads
        close(child_to_parent[0]); // child writes

        for (int i = 0; i < ITERATIONS; i++) {
            if (read(parent_to_child[0], &byte, 1) != 1) {
                perror("child read");
                exit(1);
            }

            if (write(child_to_parent[1], &byte, 1) != 1) {
                perror("child write");
                exit(1);
            }
        }

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    } else {
        // Parent process
        close(parent_to_child[0]); // parent writes
        close(child_to_parent[1]); // parent reads

        double start = now_seconds();

        for (int i = 0; i < ITERATIONS; i++) {
            if (write(parent_to_child[1], &byte, 1) != 1) {
                perror("parent write");
                return 1;
            }

            if (read(child_to_parent[0], &byte, 1) != 1) {
                perror("parent read");
                return 1;
            }
        }

        double end = now_seconds();

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        wait(NULL);

        double total_time = end - start;
        double exchanges_per_second = ITERATIONS / total_time;

        printf("Iterations: %d\n", ITERATIONS);
        printf("Total time: %.6f seconds\n", total_time);
        printf("Exchanges per second: %.2f\n", exchanges_per_second);
    }

    return 0;
}