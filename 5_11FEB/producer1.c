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

int main()
{
    signal(SIGINT, signal_handler);
    Pop.sem_num = 1;
    Pop.sem_op = -1;
    Pop.sem_flg = SEM_UNDO;

    Vop.sem_num = 0;
    Vop.sem_op = 1;
    Vop.sem_flg = SEM_UNDO;

    key_t key = ftok("producer.c", 'a');
    shmid = shmget(key, MAX_QUEUE_SIZE*sizeof(int), IPC_CREAT | 0666);
    int *queue = (int *)shmat(shmid, NULL, 0);
    int tail=0;
    int semid = semget(key, 2, IPC_CREAT | 0666);
    union Semun semun;
    semun.val = 0;
    semctl(semid, 0, SETVAL, semun); //  initialising semaphore 0 to 0
    semun.val = MAX_QUEUE_SIZE;
    semctl(semid, 1, SETVAL, semun); //  initialising semaphore 1 to MAX_QUEUE_SIZE

    srand(time(NULL));

    while (1)
    {
        P(semid); // Waiting for available space in the queue

        int item = rand() % 100; // Generating a random integer
        queue[tail] = item;
        tail = (tail + 1) % MAX_QUEUE_SIZE;

        printf("Produced item: %d\n", item);

        V(semid); // Signaling that an item has been produced
        sleep(1); 
    }

    shmdt(queue);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
