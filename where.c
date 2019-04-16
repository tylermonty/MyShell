#include <stdio.h>
#include "get_path.h"

//locate all instances of command within pathlist
char *where(char *command, struct pathelement *pathlist )
{
  char *cmd = calloc(64, sizeof(char*));
  char *test = calloc(256, sizeof(char*));
  int found = 0; //used to tell whether a path was found or not
  int built_in = 0; //used to tell whether command is built in or not

  //check to see if built in command
  if ((strcmp(command, "which") == 0) ||
      (strcmp(command, "where") == 0) ||
      (strcmp(command, "cd") == 0) ||
      (strcmp(command, "pwd") == 0) ||
      (strcmp(command, "list") == 0) ||
      (strcmp(command, "pid") == 0) ||
      (strcmp(command, "kill") == 0) ||
      (strcmp(command, "prompt") == 0) ||
      (strcmp(command, "printenv") == 0) ||
      (strcmp(command, "setenv") == 0)){
        sprintf(cmd, "%s: shell built-in command.\n", command);
        built_in = 1;
      }

  //else, locate command
  while (pathlist) {
    sprintf(test, "%s/%s", pathlist->element, command);
    if (access(test, X_OK) == 0){
      char *toAppend = malloc(64);
      sprintf(toAppend, "%s%s\n", cmd, test);
      sprintf(cmd, "%s", toAppend);
      //sprintf(cmd, "%s%s/%s\n", cmd, pathlist->element, command);
      found = 1;
      free(toAppend);
    }
    pathlist = pathlist->next;
  }
  free(test);
  if (found == 0 && built_in == 0){
    sprintf(cmd, "%s: Command not found.\n", command);
    return cmd;
  }
  return cmd;
}
