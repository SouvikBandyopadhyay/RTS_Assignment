#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
// Function to calculate the ith term of the polynomial
double evaluateTerm(int exponent, double coefficient, double x) {
    return coefficient * pow(x, exponent);
}

int main(int argc, char *argv[]) {
    // Checking if the correct number of command line arguments is provided
    if (argc < 3) {
        printf("Usage: %s x an an-1 an-2 ... a2 a1 a0\n", argv[0]);
        return 1;
    }

    // Converting command line arguments to double values
    double x = atof(argv[1]);
    double coefficients[argc - 2];

    for (int i = 2; i < argc; i++) {
        coefficients[i - 2] = atof(argv[i]);
    }

    // Number of terms in the polynomial
    int n = argc - 3;


        // Creating shared memory segment
    int shmid = shmget(1234, sizeof(int), IPC_CREAT | 0777);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attaching shared memory to the process
    int* sharedMem = (int*)shmat(shmid, NULL, 0);
    if (sharedMem == (int*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Initializing shared memory with 0 
    *sharedMem = 0;

    // Iterating through the range [0, n] to create child processes
    for (int i = 0; i <= n; i++) {
        pid_t pid = fork();

        // Checking for fork error
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Child process
        if (pid == 0) {
            // Evaluating the ith term of the polynomial
            double termValue = evaluateTerm(n - i, coefficients[i], x);
            printf("Child process %d with PID %d: Term %d evaluated to %f\n", i, getpid(), i, termValue);
            // Saving the result in shared memory
            *sharedMem=*sharedMem+termValue;
            // If it's the last child, print the value of the whole polynomial
            if (i == n) {
                printf("Child process %d with PID %d: Polynomial value for x=%f is %d\n", i, getpid(), x, *sharedMem);
            }

            exit(0);
        }

        // Parent process waits for the child to complete
        wait(NULL);
    }

    // Detaching the shared memory segment from the process
    if (shmdt(sharedMem) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // Removing the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}
