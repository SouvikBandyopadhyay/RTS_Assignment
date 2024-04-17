#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define FILENAME "teacher.c"

struct att {
    int roll;
    time_t tm;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);

    if (N <= 0) {
        printf("Invalid input. N should be a positive integer.\n");
        return 1;
    }

    key_t key = ftok(FILENAME, 'R');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    struct att *rolls = (struct att *)shmat(shmid, NULL, 0);
    if (rolls == (struct att *)(-1)) {
        perror("shmat");
        return 1;
    }

    if (rolls[N - 1].roll == -1) {
        rolls[N - 1].roll = N;
        rolls[N - 1].tm = time(NULL);
    } 

    // Detach shared memory
    if (shmdt(rolls) == -1) {
        perror("shmdt");
    }

    return 0;
}
