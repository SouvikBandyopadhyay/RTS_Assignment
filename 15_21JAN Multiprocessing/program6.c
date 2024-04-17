#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Checking if the correct number of command line arguments is provided
    if (argc < 2) {
        printf("Usage: %s executable1 param1 executable2 param2 ... executableN paramN\n", argv[0]);
        return 1;
    }

    // Iterating through the range [1, N] to create child processes
    for (int i = 1; i < argc; i=i+2) {
	
        pid_t pid = fork();

        // Checking for fork error
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Creating Child process
        if (pid == 0) {
            // Execute the ith executable file
            char *myargv[]={argv[i],argv[i+1], NULL};
            int k=execve(argv[i], myargv, NULL);

            // If execve fails
            perror("Exec failed");
            exit(1);
        }

        // Parent process waits for the child to complete
        wait(NULL);
    }

    return 0;
}

