// 2 variables pthreadno and cthreadno are maintained to keep track of how many producer and consumer thread to keep alive
// to kill a thread ... corrosponding variable value is reduced
// a thread will run if its corrosponding thread no. is less than the current (p/c)threadno of that type
// min no of threads is set to 1 of each kind and max to 5, m=2 and n=3
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
// ===============================================================================================

// The current execution workds like this:

// 2 arrays of threads are created along with a manager thread

// the manager thread takes 4 inputs on input

// 1: create a P_thread pass P_thread_number as argument and increment the P_thread_number and add to P_thread_array

// 2: decrement the P_thread_number

// 3: create a C_thread pass C_thread_number as argument and increment the C_thread_number and add to C_thread_array

// 4: decrement the C_thread_number

// The producer and consumer threads gets argemtn makes it there thread_ID
// then in an infinite loop checks if there thread_ID is less than the current_number_of_thread_of_that_type
// If true dose its JOB*
// else returns

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define queLen 10
#define maxproducer 5
#define maxconsumer 5
#define m 2
#define n 3

int pthreadno = 0, cthreadno = 0;
int queue[queLen];
int head = 0, tail = 0, count = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_t mthread;
pthread_t cthreads[maxconsumer];
pthread_t pthreads[maxproducer];

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
    int threadid = (int)arg;
    while (threadid < pthreadno)
    {
        pthread_mutex_lock(&mutex);
        while (num_free_spaces() == 0)
        {
            printf("Producer %d waiting\n", (int)arg);
            pthread_cond_wait(&cond, &mutex);
        }
        int item = rand() % 100; // Generate random item
        enqueue(item);
        printf("Produced by thread %d: %d\n", (int)arg, item);
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond);
        sleep(4);
        // getchar();
    }
    return NULL;
}

void *consumer(void *arg)
{
    int threadid = (int)arg;
    while (threadid < cthreadno)
    {
        pthread_mutex_lock(&mutex);
        while (num_filled_spaces() == 0)
        {
            printf("Consumer %d waiting\n", (int)arg);
            pthread_cond_wait(&cond, &mutex);
        }
        int item = dequeue();
        printf("Consumed by thread %d: %d\n", (int)arg, item);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
        sleep(4);
        // getchar();
    }
    return NULL;
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
            if (pthreadno < maxproducer)
            {
                // create producer
                // add to array
                if (pthread_create(&pthreads[pthreadno], NULL, producer, (void *)pthreadno) != 0)
                {
                    perror("pthread_create");
                }
                // incriment pthreadno
                pthreadno++;
                printf("increased producers to %d\n", pthreadno);
            }
            else
            {
                printf("Cannot increase producers, maximum limit reached.\n");
            }
            break;
        case 2:
            if (pthreadno > 0)
            {
                // kill producer thread
                // decriment pthreadno
                pthreadno--;
                sleep(2);
                printf("decreased producer to %d\n", pthreadno);
            }
            else
            {
                printf("Cannot decrease producers, minimum limit reached.\n");
            }
            break;
        case 3:
            if (cthreadno < maxconsumer)
            {
                // add consumer thread
                // add to array
                if (pthread_create(&cthreads[cthreadno], NULL, consumer, (void *)cthreadno) != 0)
                {
                    perror("cthread_create");
                }
                // incriment cthreadno
                cthreadno++;
                printf("increased consumers to %d\n", cthreadno);
            }
            else
            {
                printf("Cannot increase consumers, maximum limit reached.\n");
            }
            break;
        case 4:
            if (cthreadno > 0)
            {
                // kill producer thread
                // decriment pthreadno
                cthreadno--;
                sleep(2);
                printf("decreased consumers to %d\n", cthreadno);
            }
            else
            {
                printf("Cannot decrease consumers, minimum limit reached.\n");
            }
            break;
        case 0:
            // kill all threads
            pthreadno = 0;
            cthreadno = 0;
            exit(0);
        default:
            printf("Invalid choice\n");
        }
    }
}
int main()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    if (pthread_create(&mthread, NULL, manager, (void *)NULL) != 0)
    {
        perror("mthread_create");
    }
    for (int i = 0; i < m; ++i)
    {
        if (pthread_create(&pthreads[i], NULL, producer, (void *)pthreadno) != 0)
        {
            perror("pthread_create");
        }
        pthreadno++;
        sleep(2);
    }
    for (int i = 0; i < n; ++i)
    {
        if (pthread_create(&cthreads[i], NULL, consumer, (void *)cthreadno) != 0)
        {
            perror("cthread_create");
        }
        cthreadno++;
    }

    // Join threads
    for (int i = 0; i < pthreadno; ++i)
    {
        if (pthread_join(pthreads[i], NULL) != 0)
        {
            perror("pthread_join");
        }
    }
    for (int i = 0; i < cthreadno; ++i)
    {
        if (pthread_join(cthreads[i], NULL) != 0)
        {
            perror("cthread_join");
        }
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
