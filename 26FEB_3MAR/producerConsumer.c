// 2 variables current_no_of_producer_threads and current_no_of_consumer_threads are maintained to keep track of how many producer and consumer thread to keep alive
// to kill a thread ... corrosponding variable value is reduced
// a thread will run if its corrosponding thread no. is less than the current (p/c)threadno of that type
// max no of threads is set to 5, initial m=2 and n=3
// some warnings are encoundered during compilation due to type casting although program runns fine
// ==================================================================================================
// ||   MANAGER:
// ||       Enter
// ||       1 to increase producers,
// ||       2 to decrease producers,
// ||       3 to increase consumers,
// ||       4 to decrease consumers,
// ||       0 to exit:
// ||
// ===================================================================================================

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define queLen 10
#define maxproducer 5
#define maxconsumer 5
#define M 2
#define N 3

int current_no_of_producer_threads = 0, current_no_of_consumer_threads = 0;
int queue[queLen];
int head = 0, tail = 0, count = 0;
pthread_mutex_t producer_mutex, consumer_mutex;
pthread_cond_t item_produced_cond, item_consumed_cond;
pthread_t manager_thread;
pthread_t consumer_threads[maxconsumer];
pthread_t producer_threads[maxproducer];

void enqueue(int value)
{
    queue[head] = value;
    head = (head + 1) % queLen;
    count++;
}

int dequeue()
{
    int value = queue[tail];
    tail = (tail + 1) % queLen;
    count--;
    return value;
}

int num_free_spaces()
{
    return queLen - count;
}

int num_filled_spaces()
{
    return count;
}

void *producer(void *arg)
{
    int threadID = (int)arg;
    // Thread will run if ThreadID is lesser than Current no of producer threads else return
    while (threadID < current_no_of_producer_threads)
    {
        pthread_mutex_lock(&producer_mutex);
        while (num_free_spaces() == 0)
        {
            printf("Producer %d waiting\n", threadID);
            pthread_cond_wait(&item_consumed_cond, &producer_mutex);
        }
        if (threadID < current_no_of_producer_threads)
        {
            int item = rand() % 100; // Generate random item
            enqueue(item);
            printf("Produced by thread %d: %d\n", threadID, item);
        }
        pthread_mutex_unlock(&producer_mutex);
        pthread_cond_broadcast(&item_produced_cond);
        sleep(6);
        // getchar();
    }
    return NULL;
}

void *consumer(void *arg)
{
    int threadID = (int)arg;
    // Thread will run if ThreadID is lesser than Current no of consumer threads else return
    while (threadID < current_no_of_consumer_threads)
    {
        pthread_mutex_lock(&consumer_mutex);
        while (num_filled_spaces() == 0)
        {
            printf("\t\t\t\tConsumer %d waiting\n", threadID);
            pthread_cond_wait(&item_produced_cond, &consumer_mutex);
        }
        if (threadID < current_no_of_consumer_threads)
        {
            int item = dequeue();
            printf("\t\t\t\tConsumed by thread %d: %d\n", threadID, item);
        }
        pthread_mutex_unlock(&consumer_mutex);
        pthread_cond_broadcast(&item_consumed_cond);
        sleep(7);
        // getchar();
    }
    return NULL;
}

void *manager(void *arg)
{
    int choice;
    while (1)
    {
        printf("******** Enter 1 to increase producers, 2 to decrease producers,***********\n");
        printf("******** 3 to increase consumers, 4 to decrease consumers, 0 to exit: *****\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            if (current_no_of_producer_threads < maxproducer)
            {
                // create producer
                // add to array
                if (pthread_create(&producer_threads[current_no_of_producer_threads], NULL, producer, (void *)current_no_of_producer_threads) != 0)
                {
                    perror("pthread_create");
                }
                // incriment current_no_of_producer_threads
                current_no_of_producer_threads++;
                printf("\t\tincreased producers to %d\n", current_no_of_producer_threads);
            }
            else
            {
                printf("\t\tCannot increase producers, maximum limit reached.\n");
            }
            break;
        case 2:
            if (current_no_of_producer_threads > 0)
            {
                // kill producer thread
                // decriment current_no_of_producer_threads
                current_no_of_producer_threads--;
                sleep(2);
                printf("\t\tdecreased producer to %d\n", current_no_of_producer_threads);
            }
            else
            {
                printf("\t\tCannot decrease producers, minimum limit reached.\n");
            }
            break;
        case 3:
            if (current_no_of_consumer_threads < maxconsumer)
            {
                // add consumer thread
                // add to array
                if (pthread_create(&consumer_threads[current_no_of_consumer_threads], NULL, consumer, (void *)current_no_of_consumer_threads) != 0)
                {
                    perror("cthread_create");
                }
                // incriment current_no_of_consumer_threads
                current_no_of_consumer_threads++;
                printf("\t\tincreased consumers to %d\n", current_no_of_consumer_threads);
            }
            else
            {
                printf("\t\tCannot increase consumers, maximum limit reached.\n");
            }
            break;
        case 4:
            if (current_no_of_consumer_threads > 0)
            {
                // kill producer thread
                // decriment current_no_of_producer_threads
                current_no_of_consumer_threads--;
                sleep(2);
                printf("\t\tdecreased consumers to %d\n", current_no_of_consumer_threads);
            }
            else
            {
                printf("\t\tCannot decrease consumers, minimum limit reached.\n");
            }
            break;
        case 0:
            // kill all threads
            current_no_of_producer_threads = 0;
            current_no_of_consumer_threads = 0;
            exit(0);
        default:
            printf("\t\tInvalid choice\n");
        }
    }
}
int main()
{
    pthread_mutex_init(&producer_mutex, NULL);
    pthread_cond_init(&item_produced_cond, NULL);
    pthread_mutex_init(&consumer_mutex, NULL);
    pthread_cond_init(&item_consumed_cond, NULL);

    for (int i = 0; i < M; ++i)
    {
        if (pthread_create(&producer_threads[i], NULL, producer, (void *)current_no_of_producer_threads) != 0)
        {
            perror("pthread_create");
        }
        current_no_of_producer_threads++;
        sleep(2);
    }
    for (int i = 0; i < N; ++i)
    {
        if (pthread_create(&consumer_threads[i], NULL, consumer, (void *)current_no_of_consumer_threads) != 0)
        {
            perror("cthread_create");
        }
        current_no_of_consumer_threads++;
    }
    // created manager after initial producer and consumer
    if (pthread_create(&manager_thread, NULL, manager, (void *)NULL) != 0)
    {
        perror("manager_thread_create");
    }

    // Join threads
    for (int i = 0; i < current_no_of_producer_threads; ++i)
    {
        if (pthread_join(producer_threads[i], NULL) != 0)
        {
            perror("pthread_join");
        }
    }
    for (int i = 0; i < current_no_of_consumer_threads; ++i)
    {
        if (pthread_join(consumer_threads[i], NULL) != 0)
        {
            perror("cthread_join");
        }
    }
    // join manager at the end
    if (pthread_join(manager_thread, NULL) != 0)
    {
        perror("manager_join");
    }
    pthread_cond_destroy(&item_produced_cond);
    pthread_mutex_destroy(&producer_mutex);
    pthread_cond_destroy(&item_consumed_cond);
    pthread_mutex_destroy(&consumer_mutex);
}
