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
#include <stdbool.h>

typedef struct node {
  char *data;
  char *key;
  struct node *next;
} node;

typedef struct list {
  node *head;
  int size;
} list;

void listInit(list *l) {
  l->head = NULL;
  l->size = 0;
}

void listDelete(list *l, char *key) {
  node *current = l->head;
  node *prev = current;
  while (current) {
    if (strcmp(current->key, key) == 0) {
      break;
    }
    prev = current;
    current = current->next;
  }
  if (current == l->head) { 
    l->head = current->next;
    free(current->key);
    free(current->data);
    free(current);
    current = NULL;
    l->size--;
  }
  if (current != l->head && current != NULL) {
    prev->next = current->next;
    free(current->key);
    free(current->data);
    free(current);
    current = NULL;
    l->size--;
  }
}

void listInsert(list *l, char *key, char *data) {
  node *new = malloc(sizeof(node));
  new->key = malloc(strlen(key) + 1);
  snprintf(new->key, strlen(key) +1, "%s\n",key);
  new->data = malloc(strlen(data) + 1);
  snprintf(new->data, strlen(data)+1, "%s\n", data);
  new->next = l->head;
  l->size++;
  l->head = new;
}

void freeList(list *l) {
  node* ptr = l->head;
  node* tmp = NULL;
  while(ptr) {
    tmp = ptr->next;
    free(ptr->key);
    free(ptr->data);
    free(ptr);
    ptr = tmp;
  }
}

char *leadandtrail(char *str) {
  char *end;  
  while(isspace((unsigned char)*str)) {
    str++;
  }
  if(*str == 0){
    return str;
  }
  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) {
    end--;
  }
  // Write new null terminator character
  end[1] = '\0';
  return str;
}

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
  list *aliases = malloc(sizeof(list));
  listInit(aliases);
  if (argc == 1) {
    char linecop[1024];
    do {
      write(1, "mysh> ", strlen("mysh> "));
      char line[1024];
      char *fgval = fgets(line, 1024, stdin);
      if (fgval == NULL) {
	freeList(aliases);
	free(aliases);
	exit(0);
      }
      leadandtrail(line);
      snprintf(linecop, strlen(line)+1, "%s\n", line);
      trim(linecop);
      if (strlen(linecop) == 0) {
        continue;
      }
      if (strcmp(linecop, excomm) == 0) {
        freeList(aliases);
	free(aliases);
	exit(0);
      }
      if (strcmp(line, "\n") == 0) {
	continue;
      }
      if (strlen(line) > 512) {
	write(2, "Error: Command line too long (max 512)\n", strlen("Error: Command line too long (max 512)\n"));
	continue;
      }
      char* commandTokens[100];
      int outputNum = 0;
      int redirectCounter = 0;
      size_t length = strlen(line);
      size_t i = 0;
      char copy_line[length]; 
      snprintf(copy_line,strlen(line) + 1, "%s\n", line);
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
      char temp_line[100];
      snprintf(temp_line, strlen(line)+1, "%s\n", line);
      char* temp_output = strtok(temp_line,">");
      temp_output = strtok(NULL,deliminators);
      while (temp_output != NULL) {
        outputNum++;
        temp_output = strtok(NULL,deliminators);
      }
     
      // if there is not one redirect, no command/outputs, or too many outputs
      // print error message. 
      if (redirectCounter > 1 || line[0] == '>' || line[length - 1] == '>' || outputNum > 1) {
        write(2,redirectMisformat, strlen(redirectMisformat));
	continue;
      }
      bool ali = false;
      bool unali = false;
      // get all tokens and pass into array
      char* currToken = strtok(command,deliminators);
      while (currToken != NULL && tokenCounter < 100) {
	trim(currToken);
	if (strcmp(currToken, "alias") == 0 ) {
	  ali = true;
	}
	if (strcmp(currToken, "unalias") == 0) {
	  unali = true;
	}
	commandTokens[tokenCounter] = currToken;
        currToken = strtok(NULL,deliminators);
        tokenCounter++;
      }
      // if alias or unalias
      if (ali) {
        switch(tokenCounter) {
	  case 1:
	    if (aliases->size == 0) {
	      continue;
	    }
	    node *ptr = aliases->head;
	    while (ptr) {
	      write(1, ptr->key, strlen(ptr->key));
	      write(1, ptr->data, strlen(ptr->data));
	      write(1, "\n", strlen("\n"));
	      ptr = ptr->next;
	    }
	    break;
	  case 2:
	    if (aliases->size == 0) {
	      continue;
	    }
	    ptr = aliases->head;
	    while (ptr) {
	      if (strcmp(ptr->key, commandTokens[1]) == 0) {
	        write(1, ptr->key, strlen(ptr->key));
                write(1, ptr->data, strlen(ptr->data));
                write(1, "\n", strlen("\n"));
                break;
	      }
	      ptr = ptr->next;
	    }
	    break;
	  default:
	    if (strcmp(commandTokens[1], excomm) == 0 || strcmp(commandTokens[1], "alias") == 0 || strcmp(commandTokens[1], "unalias") == 0) {
              write(2, "alias: Too dangerous to alias that.\n", strlen("alias: Too dangerous to alias that.\n"));
              continue;
            }
	    ptr = aliases->head;
	    while(ptr) {
	      if (strcmp(ptr->key, commandTokens[1]) == 0) {
		listDelete(aliases, commandTokens[1]);
		break;
	      }
	      ptr = ptr->next;
	    }
	    char repval[] = " ";
	    for (int i = 2; i < tokenCounter; i++) {
	      char *tok = commandTokens[i];
	      strcat(repval, tok);
	      strcat(repval, " ");
	    }
	    leadandtrail(repval);
	    listInsert(aliases, commandTokens[1], repval);
	}
        continue;
      }
      if (unali) {
	if (tokenCounter != 2) {
	  write(2, "unalias: Incorrect number of arguments.\n", strlen("unalias: Incorrect number of arguments.\n"));
	  continue;
	}
	listDelete(aliases, commandTokens[1]);
	continue;
      }
      node *comptr = aliases->head;
      while (comptr) {
	if (strcmp(comptr->key, commandTokens[0]) == 0) {
	  tokenCounter = 0;
	  currToken = strtok(comptr->data,deliminators);
          while (currToken != NULL && tokenCounter < 100) {
            trim(currToken);
            commandTokens[tokenCounter] = currToken;
            currToken = strtok(NULL,deliminators);
            tokenCounter++;
          }
	  break;
	}
	comptr = comptr->next;
      }
      //end alias
      commandTokens[tokenCounter] = NULL; 
      // after all cases passed, execute the function
      pid_t pid = fork(); // create a new child process
      if (pid == -1) {
        write(2, "Fork failed.", strlen("Fork failed"));
	freeList(aliases);
	free(aliases);
	exit(1);
      }
      if (pid == 0) { // this is child
      // do some preparation
	if (redirectCounter == 1) {
	  int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (fd == -1) {
            char* openError = strcat("Error: Cannot open file",output);
            openError = strcat(openError, "\n");
            write(2, openError, strlen(openError));
            continue;
          }
          dup2(fd,fileno(stdout));
          close(fd);
	}
	execv(commandTokens[0],commandTokens);
        // print error message.
        char* execError = strcat(commandTokens[0], ": ");
        execError = strcat(execError,"Command not found.\n");
        write(2, execError, strlen(execError));
	freeList(aliases);
	free(aliases);
	exit(1); // this means execv() fails
      }
      // parent
      int status;
      waitpid(pid, &status, 0); // wait the child to finish its work before keep going
      // continue to handle the next command
    } while (strcmp(linecop, excomm) != 0);
    freeList(aliases);
    free(aliases);
    exit(0);
  }
  else if (argc == 2) {
    FILE *fp = fopen(argv[1], "r");
    FILE *temp = fopen(argv[1], "r");
    if (fp == NULL || temp == NULL) {
      write(2, "Error: Cannot open file ", strlen("Error: Cannot open file "));
      write(2, argv[1], strlen(argv[1]));
      write(2, ".\n", strlen(".\n"));
      freeList(aliases);
      free(aliases);
      exit(1);
    }
    char linecop[4096];
    char line[4096];
    while (fgets(line, 4096, fp)) {
      if (strlen(line) > 512) {
	write(1, line, 512);
	write(2, "Error: Command line too long (max 512)\n", strlen("Error: Command line too long (max 512)\n"));
	continue;
      }
      else {
        write(1, line, strlen(line));
      }
      leadandtrail(line);
      snprintf(linecop, strlen(line)+1, "%s\n", line);
      trim(linecop);
      if (strlen(linecop) == 0) {
	continue;
      }
      if (strcmp(linecop, excomm) == 0) {
	fclose(fp);
        fclose(temp);	
	freeList(aliases);
	free(aliases);
	exit(0);
      }
      int outputNum = 0;
      int redirectCounter = 0;
      size_t length = strlen(line);
      size_t i = 0;
      char copy_line[length];
      snprintf(copy_line,strlen(line) + 1, "%s\n", line);
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
      char temp_line[4096];
      snprintf(temp_line, strlen(line)+1, "%s\n", line);
      char* temp_output = strtok(temp_line,">");
      temp_output = strtok(NULL,deliminators);
      while (temp_output != NULL) {
        outputNum++;
        temp_output = strtok(NULL,deliminators);
      }
     
      // if there is not one redirect, no command/outputs, or too many outputs
      // print error message.
      if (redirectCounter > 1 || line[0] == '>' || line[length - 1] == '>' || outputNum > 1) {
        write(2,redirectMisformat, strlen(redirectMisformat));
	continue;
      }
    
      bool ali = false;
      bool unali = false;
      // get all tokens and pass into array
      char* currToken = strtok(command,deliminators);
      while (currToken != NULL && tokenCounter < 100) {
	trim(currToken);
	if (strcmp(currToken, "alias") == 0 ) {
	  ali = true;
	}
	if (strcmp(currToken, "unalias") == 0) {
	  unali = true;
	}
	commandTokens[tokenCounter] = currToken;
        currToken = strtok(NULL,deliminators);
        tokenCounter++;
      }
      // if alias or unalias
      if (ali) {
        switch(tokenCounter) {
	  case 1:
	    if (aliases->size == 0) {
	      continue;
	    }
	    node *ptr = aliases->head;
	    while (ptr) {
	      write(1, ptr->key, strlen(ptr->key));
	      write(1, ptr->data, strlen(ptr->data));
	      write(1, "\n", strlen("\n"));
	      ptr = ptr->next;
	    }
	    break;
	  case 2:
	    if (aliases->size == 0) {
	      continue;
	    }
	    ptr = aliases->head;
	    while (ptr) {
	      if (strcmp(ptr->key, commandTokens[1]) == 0) {
	        write(1, ptr->key, strlen(ptr->key));
                write(1, ptr->data, strlen(ptr->data));
                write(1, "\n", strlen("\n"));
                break;
	      }
	      ptr = ptr->next;
	    }
	    break;
	  default:
	    if (strcmp(commandTokens[1], excomm) == 0 || strcmp(commandTokens[1], "alias") == 0 || strcmp(commandTokens[1], "unalias") == 0) {
              write(2, "alias: Too dangerous to alias that.\n", strlen("alias: Too dangerous to alias that.\n"));
              continue;
            }
	    ptr = aliases->head;
	    while(ptr) {
	      if (strcmp(ptr->key, commandTokens[1]) == 0) {
		listDelete(aliases, commandTokens[1]);
		break;
	      }
	      ptr = ptr->next;
	    }
	    char repval[] = " ";
	    for (int i = 2; i < tokenCounter; i++) {
	      char *tok = commandTokens[i];
	      strcat(repval, tok);
	      strcat(repval, " ");
	    }
	    leadandtrail(repval);
	    listInsert(aliases, commandTokens[1], repval);
	}
        continue;
      }
      if (unali) {
	if (tokenCounter != 2) {
	  write(2, "unalias: Incorrect number of arguments.\n", strlen("unalias: Incorrect number of arguments.\n"));
	  continue;
	}
	listDelete(aliases, commandTokens[1]);
	continue;
      }
      node *comptr = aliases->head;
      while (comptr) {
	if (strcmp(comptr->key, commandTokens[0]) == 0) {
	  tokenCounter = 0;
	  currToken = strtok(comptr->data,deliminators);
          while (currToken != NULL && tokenCounter < 100) {
            trim(currToken);
            commandTokens[tokenCounter] = currToken;
            currToken = strtok(NULL,deliminators);
            tokenCounter++;
          }
	  break;
	}
	comptr = comptr->next;
      }
      //end alias
      commandTokens[tokenCounter] = NULL;
      // after all cases passed, execute the function
      pid_t pid = fork(); // create a new child process
      if (pid == -1) {
        write(2, "Fork failed.", strlen("Fork failed"));
	fclose(fp);
        fclose(temp);	
	freeList(aliases);
	free(aliases);
	exit(1);
      }
      if (pid == 0) { // this is child
      // do some preparation
	if (redirectCounter == 1) {
          int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (fd == -1) {
            char* openError = strcat("Error: Cannot open file",output);
            openError = strcat(openError, "\n");
            write(2, openError, strlen(openError));
            continue;
          }
          dup2(fd,fileno(stdout));
          close(fd);
	}
	execv(commandTokens[0],commandTokens);
        // print error message.
        char* execError = strcat(commandTokens[0], ": ");
        execError = strcat(execError,"Command not found.\n");
	write(2, execError, strlen(execError));
	char tempLine[4096];
	char endLine[4096];
	while (fgets(tempLine, 4096, temp)) {
	  if (feof(temp)) {
	    break;
	  }
	  snprintf(endLine, strlen(tempLine)+1, "%s\n", tempLine);
	}
	leadandtrail(endLine);
	if (strcmp(endLine, line) == 0) {
	  freeList(aliases);
	  free(aliases);
	  fclose(temp);
	  fclose(fp);
	}
	exit(1); // this means execv() fails
      }
      // parent
      int status;
      waitpid(pid, &status, 0); // wait the child to finish its work before keep goingy
      // continue to handle the next command
    }
    fclose(temp);
    fclose(fp);
  }
  else {
    write(2, "Usage: mysh [batch-file]\n", strlen("Usage: mysh [batch-file]\n"));
    freeList(aliases);
    free(aliases);
    exit(1);
  }
  freeList(aliases);
  free(aliases);
  return 0;
}
