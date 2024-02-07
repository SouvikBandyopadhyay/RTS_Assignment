#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define SHM_SIZE 1024
#define MAX_NUM 10
#define MIN_NUM 1

int *shared_memory;
int shmid;
pid_t pid;

// Function to calculate factorial
int factorial(int num)
{
    if (num == 0 || num == 1)
    {
        return 1;
    }
    else
    {
        return num * factorial(num - 1);
    }
}

void child_handler(int signum)
{
}

void parent_handler(int signum)
{
}

void termination_handler(int signum)
{
    // Detach the shared memory segment
    shmdt(shared_memory);

    // Delete the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);
    if (getpid() == pid)
    {
        kill(getppid(), SIGKILL);
        _exit(0);
    }
    else
    {
        kill(pid, SIGKILL);
        _exit(0);
    }
}

int main()
{
    key_t key;

    // Generate a key for shared memory
    key = ftok("program1.c", 1);
    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    // Create the shared memory segment
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    // Attach the shared memory segment
    shared_memory = (int *)shmat(shmid, NULL, 0);
    if (shared_memory == (int *)(-1))
    {
        perror("shmat");
        exit(1);
    }

    // Initialize the shared memory to zero
    *shared_memory = 0;

    // Set signal handlers
    __sighandler_t a1 = signal(SIGUSR1, parent_handler);
    __sighandler_t a2 = signal(SIGUSR2, child_handler);
    __sighandler_t a3 = signal(SIGINT, termination_handler);

    // Fork a child process
    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }
    else if (pid == 0)
    { // Child process
        pid_t ppid = getppid();
        while (1)
        {
            int i = pause();
            *shared_memory = factorial(*shared_memory);
            kill(ppid, SIGUSR1);
        }
    }
    else
    { // Parent process
        while (1)
        {
            printf("Factorial of ");
            *shared_memory = (int)rand() % (MAX_NUM + 1 - MIN_NUM) + MIN_NUM;
            printf("%d ", *shared_memory);
            kill(pid, SIGUSR2);
            int i = pause();
            printf("is %d\n", *shared_memory);
            sleep(1);
            // getchar();
        }
    }

    // Detach the shared memory segment
    shmdt(shared_memory);

    // Delete the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
