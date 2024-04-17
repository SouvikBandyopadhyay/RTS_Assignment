#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX_ROWS 100
#define MAX_COLS 100

int main() {

    // Storing the arrays from user
    int A[MAX_ROWS][MAX_COLS], B[MAX_ROWS][MAX_COLS];
    int rows1, cols1, cols2;

    printf("Enter rows(N) for first matrix: ");
    scanf("%d", &rows1);
    
    printf("Enter columns(M) for first matrix: ");
    scanf("%d", &cols1);

    printf("Enter elements of first matrix:\n");
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols1; j++) {
            scanf("%d", &A[i][j]);
        }
    }

    printf("Enter columns(P) for second matrix: ");
    scanf("%d", &cols2);


    printf("Enter elements of second matrix:\n");
    for (int i = 0; i < cols1; i++) {
        for (int j = 0; j < cols2; j++) {
            scanf("%d", &B[i][j]);
        }
    }

    // Creating two arrays one for storing shared memory IDs and another for storing shared memory addresses for C matrix
    int shmid[MAX_ROWS];
    // array C contains N elements each element is the address to a shared memory of atleast P size
    int *C[MAX_ROWS];
    for(int i=0; i<rows1;i++){
        shmid[i]=shmget(IPC_PRIVATE, sizeof(int) * cols2, 0666 | IPC_CREAT);
        if (shmid[i] == -1) {
            perror("shmget");
            exit(1);
        }
        C[i] = (int *)shmat(shmid[i], 0, 0);
        if (C[i] == (int *)(-1)) {
            perror("shmat");
            exit(1);
        }
    }

    // Creating N child processes
    for (int i = 0; i < rows1; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) { // In each child process
        // calculating ith row
            for (int j = 0; j < cols2; j++) {
                int sum = 0;
                for (int k = 0; k < cols1; k++) {
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
            exit(0);
        }
    }

    // Waiting for all child processes to terminate
    for (int i = 0; i < rows1; i++) {
        wait(NULL);
    }

    // Printing the C matrix
    printf("Product of the matrices:\n");
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols2; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }

    // Detaching and removing the shared memories C
    for(int i=0; i<rows1;i++){
        shmdt(C[i]);
        shmctl(shmid[i], IPC_RMID, NULL);
    }
    return 0;
}
