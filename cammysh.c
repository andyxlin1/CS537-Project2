#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

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
      char linebuff[1024];
      char *fgval = fgets(linebuff, 1024, stdin);
      if (fgval == NULL) {
        exit(0);
      }
      snprintf(linecop, strlen(linebuff)+1, "%s\n", linebuff);
      trim(linecop);
      if (strcmp(linebuff, "\n") == 0) {
	continue;
      }
      if (strlen(linebuff) > 512) {
	write(2, "Error: Command line too long (max 512)\n", strlen("Error: Command line too long (max 512)\n"));
	continue;
      }
      //do command
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
  }
  else {
    write(2, "Usage: mysh [batch-file]\n", strlen("Usage: mysh [batch-file]\n"));
    exit(1);
  }
  return 0;

}
