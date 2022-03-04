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
char deliminators[] = " \t\n\r";
char* commandTokens[100];

void trim(char *string) {
  int non_space_count = 0;
 
  for (int i = 0; string[i] != '\0'; i++) {
    if (isspace(string[i]) == 0) {
      string[non_space_count] = string[i];
      non_space_count++;
    }    
  }
  string[non_space_count] = '\0';
}

int main(int argc, char *argv[]) {
  const char *excomm = "exit";
  if (argc == 1) {
    char linecop[1024];
    do {
      write(1, "mysh> ", strlen("mysh> "));
      char line[1024];
      char *fgval = fgets(line, 1024, stdin);
      if (fgval == NULL) {
        exit(0);
      }
      snprintf(linecop, strlen(line)+1, "%s\n", line);
      trim(linecop);
      if (strcmp(linecop, excomm) == 0) {
	exit(0);
      }
      if (strcmp(line, "\n") == 0) {
	continue;
      }
      if (strlen(line) > 512) {
	write(2, "Error: Command line too long (max 512)\n", strlen("Error: Command line too long (max 512)\n"));
	continue;
      }
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
      if (redirectCounter > 1 || redirectCounter < 1 || copy_line[0] == '>' || copy_line[length - 1] == '>' || outputNum > 1) {
        write(2,redirectMisformat, strlen(redirectMisformat));
	continue;
      }
    
      // get all tokens and pass into array
      char* currToken = strtok(command,deliminators);
      while (currToken != NULL && tokenCounter < 100) {
	trim(currToken);
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
            continue;
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
    } while (strcmp(linecop, excomm) != 0);
    exit(0);
  }
  else if (argc == 2) {
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
      write(2, "Error: Cannot open file ", strlen("Error: Cannot open file "));
      write(2, argv[1], strlen(argv[1]));
      write(2, ".\n", strlen(".\n"));
      exit(1);
    }
    char linecop[1024];
    char line[1024];
    while (fgets(line, 1024, fp)) {
      if (strlen(line) > 512) {
	write(1, line, 512);
	write(2, "Error: Command line too long (max 512)\n", strlen("Error: Command line too long (max 512)\n"));
      }
      else {
        write(1, line, strlen(line));
      }
      snprintf(linecop, strlen(line)+1, "%s\n", line);
      trim(linecop);
      if (strcmp(linecop, excomm) == 0) {
        exit(0);
      }
    }
  }
  else {
    write(2, "Usage: mysh [batch-file]\n", strlen("Usage: mysh [batch-file]\n"));
    exit(1);
  }
  return 0;
}
