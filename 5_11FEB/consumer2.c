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


int main()
{
    Pop.sem_num = 0;
    Pop.sem_op = -1;
    Pop.sem_flg = SEM_UNDO;

    Vop.sem_num = 1;
    Vop.sem_op = 1;
    Vop.sem_flg = SEM_UNDO;

    key_t key = ftok("producer.c", 'a');
    shmid = shmget(key, sizeof(struct CircularQueue), 0666);
    struct CircularQueue *queue = (struct CircularQueue *)shmat(shmid, NULL, 0);

    int semid = semget(key, 2, 0666);

    while (1)
    {
        P(semid); // Waiting for an item to be produced

        int item = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % MAX_QUEUE_SIZE;

        printf("Consumed item: %d\n", item);

        V(semid); // Signaling that space is available in the queue
        sleep(1); 
    }

    shmdt(queue);
    return 0;
}
