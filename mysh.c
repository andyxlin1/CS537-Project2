#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>

char* redirectMisformat = "Redirection misformatted.\n";
char deliminators[] = "\t\n\r";
char* commandTokens[100];
void runCommand (char* line) {
    int outputNum = 0;
    int redirectCounter = 0;
    size_t length = strlen(line);
    size_t i = 0;
    char* copy_line = strdup(line);
    char* command = strtok(copy_line, ">");
    char* output = strtok(NULL, deliminators);
    int tokenCounter = 0;
    
    // get number of redirections
    for (; i < length; i++) {
        if (line[i] == '>') {
            redirectCounter++;
        }
    }

    // get number of outputs
    char* temp_line = strdup(line);
    char* temp_output = strtok(temp_line,">");
    temp_output = strtok(NULL,deliminators);
    while (temp_output != NULL) {
        outputNum++;
        temp_output = strtok(NULL,deliminators);
    }
    
    // if there is not one redirect, no command/outputs, or too many outputs
    // print error message.
    if (redirectCounter > 1 || redirectCounter < 1 || copy_line[0] == '>'
    || copy_line[length - 1] == '>' || outputNum > 1) {
        write(2,redirectMisformat, strlen(redirectMisformat));
    }
    
    // get all tokens and pass into array
    char* currToken = strtok(command,deliminators);
    while (currToken != NULL && tokenCounter < 100) {
        commandTokens[tokenCounter] = currToken;
        currToken = strtok(NULL,deliminators);
        tokenCounter++;
    } 
    // after all cases passed, execute the function
    pid_t pid = fork(); // create a new child process
    if (pid == -1) {
        write(2, "Fork failed.", strlen("Fork failed"));
        exit(1);
    }
    if (pid == 0) { // this is child
    // do some preparation
        int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            char* openError = strcat("Error: Cannot open file",output);
            openError = strcat(openError, "\n");
            write(2, openError, strlen(openError));
           // continue;
        }
        dup2(fd,fileno(stdout));
        close(fd);
        execv(commandTokens[0],commandTokens);
        // print error message.
        char* execError = strcat(commandTokens[0], ": ");
        execError = strcat(execError,"Command not found.\n");
        write(2, execError, strlen(execError));
        exit(1); // this means execv() fails
}
    // parent
    int status;
    waitpid(pid, &status, 0); // wait the child to finish its work before keep going
    // continue to handle the next command

}

int main() {

}
