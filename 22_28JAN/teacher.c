#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#define FILENAME "teacher.c"

struct att {
    int roll;
    time_t tm;
};


int shmid = -1; // Global variable to store shared memory ID
int N;
struct att *rolls;
struct tm *timeinfo;

void sort_att_array(struct att *att_array, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (att_array[j].tm > att_array[j + 1].tm) {
                // Swap elements
                struct att temp = att_array[j];
                att_array[j] = att_array[j + 1];
                att_array[j + 1] = temp;
            }
        }
    }
}

void signal_handler(int signum) {
    if (signum == SIGINT) {
        // Calling the sorting function to sort the array in chronological order of the attendance
        sort_att_array(rolls, N);
        // Printing the attendence 
        for(int i=0;i<N;i++){
            if(rolls[i].roll!=-1){
                char formatted_time[50]; // Buffer to store formatted time
                timeinfo = localtime(&rolls[i].tm);
                strftime(formatted_time, sizeof(formatted_time), "%B %d, %Y, %H:%M", timeinfo);
                printf("\nStudent with roll number %d marked attendance at s%s", rolls[i].roll,formatted_time);
            }
        }
        // Detaching shared memory
        if (shmdt(shmat(shmid, NULL, 0)) == -1) {
            perror("shmdt");
        }

        // Deleting shared memory segment
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
        }

        printf("\nShared memory detached and deleted.\n");
        exit(0);
    }
}


int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);

    if (N <= 0) {
        printf("Invalid input. N should be a positive integer.\n");
        return 1;
    }

    int key = ftok(FILENAME, 'R');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, N * sizeof(struct att), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    rolls = (struct att *)shmat(shmid, NULL, 0);
    if (rolls == (struct att *)(-1)) {
        perror("shmat");
        return 1;
    }

    // Populate the shared memory array with -1 initially
    for (int i = 0; i < N; i++) {
        rolls[i].roll = -1;
        rolls[i].tm = time(NULL);
    }

    pause();

    return 0;
}
