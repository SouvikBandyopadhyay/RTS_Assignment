#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_SIZE 100
#define RAND_RANGE 100

void fillRandomMatrix(int rows, int cols, int matrix[MAX_SIZE][MAX_SIZE]) {
    srand(time(NULL));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() % RAND_RANGE;
        }
    }
}

int main() {
    int m, n, r;

    // Building a random matrix A
    printf("Enter the number of rows (m) for matrix A: ");
    scanf("%d", &m);
    printf("Enter the number of columns (n) for matrix A: ");
    scanf("%d", &n);

    int matrixA[MAX_SIZE][MAX_SIZE];
    fillRandomMatrix(m, n, matrixA);

    // Building random matrix B
    printf("Enter the number of columns (r) for matrix B: ");
    scanf("%d", &r);

    int matrixB[MAX_SIZE][MAX_SIZE];
    fillRandomMatrix(n, r, matrixB);

    // For product matrix C
    // int matrixC[MAX_SIZE][MAX_SIZE];


        // Creating shared memory segment
    int shmid = shmget(1234, m*r*sizeof(int), IPC_CREAT | 0777);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attaching shared memory to the process
    int* matrixC = (int*)shmat(shmid, NULL, 0);
    if (matrixC == (int*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Initializing shared memory with 0 
    // *matrixC = 0;

    // Creating m*r child processes
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            pid_t pid = fork();

            // Checking for fork error
            if (pid < 0) {
                perror("Fork failed");
                exit(1);
            }

            // Creating child process
            if (pid == 0) {
                // Computing the product element and communicate as status
                int productElement = 0;
                for (int k = 0; k < n; k++) {
                    productElement += matrixA[i][k] * matrixB[k][j];
                }
                matrixC[i*m+j]=productElement;
                exit(productElement);
            }
        }
    }



    // Print the product matrix
    printf("Matrix A (%d x %d):\n", m, n);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d\t", matrixA[i][j]);
        }
        printf("\n");
    }

    printf("\nMatrix B (%d x %d):\n", n, r);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < r; j++) {
            printf("%d\t", matrixB[i][j]);
        }
        printf("\n");
    }

    printf("\nProduct Matrix (A X B) (%d x %d):\n", m, r);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            int status;
            wait(&status);
            printf("%d\t", matrixC[i*m+j]);
        }
        printf("\n");
    }
    // Detaching the shared memory segment from the process
    if (shmdt(matrixC) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // Removing the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}
