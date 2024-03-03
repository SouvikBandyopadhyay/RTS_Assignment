
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define MAX_PRODUCERS 5
#define MAX_CONSUMERS 5

// Circular queue structure
typedef struct
{
    int items[BUFFER_SIZE];
    int head;
    int tail;
} CircularQueue;

CircularQueue queue;

int num_producers = 2;
int num_consumers = 2;
int exit_producers = 0;
int exit_consumers = 0;

sem_t mutex, full, empty;

void initQueue()
{
    queue.head = 0;
    queue.tail = 0;
}

void enqueue(int item)
{
    queue.items[queue.head] = item;
    queue.head = (queue.head + 1) % BUFFER_SIZE;
}

int dequeue()
{
    int item = queue.items[queue.tail];
    queue.tail = (queue.tail + 1) % BUFFER_SIZE;
    return item;
}

void *producer(void *arg)
{
    int item = 0;
    while (!exit_producers)
    {
        sem_wait(&empty);
        sem_wait(&mutex);

        item++; // Producing a new item
        enqueue(item);
        printf("Produced: %d\n", item);

        sem_post(&mutex);
        sem_post(&full);
        sleep(1);
    }
    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    int item;
    while (!exit_consumers)
    {
        sem_wait(&full);
        sem_wait(&mutex);

        item = dequeue();
        printf("Consumed: %d\n", item);

        sem_post(&mutex);
        sem_post(&empty);
        sleep(2);
    }
    pthread_exit(NULL);
}

void *manager(void *arg)
{
    int choice;
    while (1)
    {
        printf("Enter 1 to increase producers, 2 to decrease producers,\n");
        printf("3 to increase consumers, 4 to decrease consumers, 0 to exit: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            if (num_producers < MAX_PRODUCERS)
            {
                num_producers++;
                pthread_t new_producer;
                pthread_create(&new_producer, NULL, producer, NULL);
            }
            else
            {
                printf("Cannot increase producers, maximum limit reached.\n");
            }
            break;
        case 2:
            if (num_producers > 0)
            {
                num_producers--;
                exit_producers = 1;
                // Allowing some time for producers to exit
                sleep(2);
                exit_producers = 0;
            }
            else
            {
                printf("Cannot decrease producers, minimum limit reached.\n");
            }
            break;
        case 3:
            if (num_consumers < MAX_CONSUMERS)
            {
                num_consumers++;
                pthread_t new_consumer;
                pthread_create(&new_consumer, NULL, consumer, NULL);
            }
            else
            {
                printf("Cannot increase consumers, maximum limit reached.\n");
            }
            break;
        case 4:
            if (num_consumers > 0)
            {
                num_consumers--;
                exit_consumers = 1;
                // Allowing some time for consumers to exit
                sleep(2);
                exit_consumers = 0;
            }
            else
            {
                printf("Cannot decrease consumers, minimum limit reached.\n");
            }
            break;
        case 0:
            exit_producers = 1;
            exit_consumers = 1;
            // Allowing some time for threads to exit
            sleep(2);
            printf("Exiting...\n");
            exit(0);
        default:
            printf("Invalid choice\n");
        }
    }
}

int main()
{
    pthread_t producers[MAX_PRODUCERS], consumers[MAX_CONSUMERS], mgr_thread;

    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);

    initQueue();

    // Creating initial producer threads
    for (int i = 0; i < num_producers; i++)
    {
        pthread_create(&producers[i], NULL, producer, NULL);
    }

    // Creating initial consumer threads
    for (int i = 0; i < num_consumers; i++)
    {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    // Creating manager thread
    pthread_create(&mgr_thread, NULL, manager, NULL);

    // Joining threads
    for (int i = 0; i < num_producers; i++)
    {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < num_consumers; i++)
    {
        pthread_join(consumers[i], NULL);
    }
    pthread_join(mgr_thread, NULL);

    // Destroying semaphores
    sem_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}
