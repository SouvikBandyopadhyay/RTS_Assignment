#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <signal.h>

#define MAX_QUEUE_SIZE 10
#define P(s) semop(s, &Pop, 1);
#define V(s) semop(s, &Vop, 1);

struct sembuf Pop;
struct sembuf Vop;

struct CircularQueue
{
    int buffer[MAX_QUEUE_SIZE];
    int head;
    int tail;
};

union Semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int shmid;
void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        // Detaching shared memory
        if (shmdt(shmat(shmid, NULL, 0)) == -1)
        {
            perror("shmdt");
        }

        // Deleting shared memory segment
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
            perror("shmctl");
        }

        printf("\nShared memory detached and deleted.\n");
        exit(0);
    }
}

// Function to perform semaphore P (wait) operation
void sem_wait(int semid) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(semid, &sem_op, 1);
}

// Function to perform semaphore V (signal) operation
void sem_signal(int semid) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    semop(semid, &sem_op, 1);
}

int main()
{
    signal(SIGINT, signal_handler);
    Pop.sem_num = 1;
    Pop.sem_op = -1;
    Pop.sem_flg = SEM_UNDO;

    Vop.sem_num = 0;
    Vop.sem_op = 1;
    Vop.sem_flg = SEM_UNDO;

    key_t key = ftok("producer2.c", 'a');
    key_t key2 = ftok("producer2.c", 'b');
    shmid = shmget(key, sizeof(struct CircularQueue), IPC_CREAT | 0666);
    struct CircularQueue *queue = (struct CircularQueue *)shmat(shmid, NULL, 0);

    int semid = semget(key, 2, IPC_CREAT | 0666);
    union Semun semun;
    semun.val = 0;
    semctl(semid, 0, SETVAL, semun); //  initialising semaphore 0 to 0
    semun.val = MAX_QUEUE_SIZE;
    semctl(semid, 1, SETVAL, semun); //  initialising semaphore 1 to MAX_QUEUE_SIZE


    // Creating semaphore
    int sem_id = semget(key2, 1, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    // Initialising semaphore value to 1
    union Semun arg;
    arg.val = 1;
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        P(semid); // Waiting for available space in the queue
        // Entering critical section
        sem_wait(sem_id);  
        int item = rand() % 100; // Generating a random integer
        queue->buffer[queue->tail] = item;
        queue->tail = (queue->tail + 1) % MAX_QUEUE_SIZE;

        printf("Produced item: %d\n", item);
        // Exiting critical section
        sem_signal(sem_id);
        V(semid); // Signaling that an item has been produced
        sleep(2); 
    }

    shmdt(queue);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
