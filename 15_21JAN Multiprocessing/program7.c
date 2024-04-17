#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Checking if the correct number of command line arguments is provided
    if (argc < 2) {
        printf("Usage: %s executable1 executable2 ... executableN\n", argv[0]);
        return 1;
    }
    pid_t pids[argc];
    int statuses[argc];
    // Iterating through the range [1, N] to create child processes
    for (int i = 1; i < argc; i++) {
        pid_t pid = fork();

        // Checking for fork error
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Creating child process
        if (pid == 0) {
            // Executing the ith executable file
            char *myargv[]={"", NULL};
            int k=execve(argv[i], myargv, NULL);

            // If execve fails
            perror("Exec failed");
            exit(1);
        }

	pids[i] = waitpid(-1, &statuses[i], 0);

    }


    for (int i = 1; i < argc; i++) {
        int status=statuses[i];

        pid_t childPid = pids[i];

        // Printing the process ID and termination status
        if (WEXITSTATUS(status)==0) {
            printf("Child process %d (PID %d) terminated normally with status reported %d\n", i, childPid, WEXITSTATUS(status));
        } else if (WEXITSTATUS(status)!=0) {
            printf("Child process %d (PID %d) terminated abnormally by signal %d\n", i, childPid, WEXITSTATUS(status));
        } else {
            printf("Child process %d (PID %d) terminated with unknown status\n", i, childPid);
        }
    }

    return 0;
}

