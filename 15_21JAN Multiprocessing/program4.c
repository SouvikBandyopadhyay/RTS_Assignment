#include <stdio.h> /* needed also for perror() */
#include <errno.h> /* needed  for perror() */
#include <unistd.h> /* needed for execve() */

int main(int argc, char *argv[]) {
        int status;
        char *myargv[]={"forkN.out", NULL};
        status = execve("forkN.out", myargv, NULL);
        if (status == -1) {
                perror ("Exec Fails: ");
        }

}