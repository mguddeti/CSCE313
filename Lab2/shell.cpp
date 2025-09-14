/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <sys/wait.h> // wait
#include <unistd.h>   // pipe, fork, dup2, execvp, close
using namespace std;

int main() {
    // lists all the files in the root directory in the long format
    char *cmd1[] = {(char *)"ls", (char *)"-al", (char *)"/", nullptr};
    // translates all input from lowercase to uppercase
    char *cmd2[] = {(char *)"tr", (char *)"a-z", (char *)"A-Z", nullptr};

    // TODO: add functionality
    // Create pipe

    // Create child to run first command
    // In child, redirect output to write end of pipe
    // Close the read end of the pipe on the child side.
    // In child, execute the command

    // Create another child to run second command
    // In child, redirect input to the read end of the pipe
    // Close the write end of the pipe on the child side.
    // Execute the second command.

    // Reset the input and output file descriptors of the parent.
    int fd[2];
    pipe(fd);

    if (fork() == 0) { // child 1
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execvp(cmd1[0], cmd1);
    }

    if (fork() == 0) { // child 2
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execvp(cmd2[0], cmd2);
    }

    close(fd[0]);
    close(fd[1]);
    wait(0);
    wait(0);
    return 0;
}
