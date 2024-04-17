#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// Function to calculate the factorial of a given number
int factorial(int num) {
    if (num == 0 || num == 1) {
        return 1;
    } else {
        return num * factorial(num - 1);
    }
}

int main(int argc, char *argv[]) {
    // Checking if the correct number of command line arguments is provided
    if (argc != 3) {
        printf("Usage: %s n1 n2\n", argv[0]);
        return 1;
    }

    // Converting command line arguments to integers
    int n1 = atoi(argv[1]);
    int n2 = atoi(argv[2]);

    // Checking if n1 is less than or equal to n2
    if (n1 > n2) {
        printf("Error: n1 should be less than or equal to n2.\n");
        return 1;
    }

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

    // Initializing shared memory with 1 (0! = 1)
    *sharedMem = 1;
    
    // Iterating through the range [n1, n2] to create child processes
    for (int i = n1; i <= n2; i++) {
        pid_t pid = fork();

        // Checking for fork error
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Child process
        if (pid == 0) {
            int result;
            // Calculating and printing the factorial of i using the previous factorial
            if(i==n1){
		        result=factorial(i);	
            }
            else{
                result = *sharedMem * i;
                }
	        printf("Child process %d with PID %d: Factorial of %d is %d\n", i - n1 + 1, getpid(), i, result);
            *sharedMem = result;
            exit(1);
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
