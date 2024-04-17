#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Function to calculate factorial
int factorial(int num) {
    if (num == 0 || num == 1) {
        return 1;
    } else {
        return num * factorial(num - 1);
    }
}

int main(int argc, char *argv[]) {

    int n = atoi(argv[1]);

    for (int i = 1; i <= n; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 0;
        } else if (pid == 0) {
            // Child process
            printf("Child process %d (PID %d): Factorial of %d is %d\n", i, getpid(), i, factorial(i));
            exit(1);
        }
    }
	wait(NULL);
    return 1;
}

