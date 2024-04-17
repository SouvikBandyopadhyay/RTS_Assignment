#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
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
    Pop.sem_num = 0;
    Pop.sem_op = -1;
    Pop.sem_flg = SEM_UNDO;

    Vop.sem_num = 1;
    Vop.sem_op = 1;
    Vop.sem_flg = SEM_UNDO;

    key_t key = ftok("producer2.c", 'a');
    key_t key2 = ftok("consumer2.c", 'b');
    shmid = shmget(key, sizeof(struct CircularQueue), 0666);
    struct CircularQueue *queue = (struct CircularQueue *)shmat(shmid, NULL, 0);

    int semid = semget(key, 2, 0666);

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
        P(semid); // Waiting for an item to be produced
        // Entering critical section
        sem_wait(sem_id);  
        int item = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % MAX_QUEUE_SIZE;

        printf("Consumed item: %d\n", item);
        // Exiting critical section
        sem_signal(sem_id);
        V(semid); // Signaling that space is available in the queue
        sleep(1); 
    }

    shmdt(queue);
    return 0;
}
