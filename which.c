#include <stdio.h>
#include "get_path.h"

//locate first instance of command within pathlist
char *which(char *command, struct pathelement *pathlist )
{
  char *cmd = malloc(64);
  int found = 0; //used to tell whether a path was found or not

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
        return cmd;
      }

  //else, locate command
  while (pathlist) {
    sprintf(cmd, "%s/%s", pathlist->element, command);
    if (access(cmd, X_OK) == 0) {
      sprintf(cmd, "%s/%s\n", pathlist->element, command);
      found = 1;
      break;
    }
    pathlist = pathlist->next;
  }
  //check if command not found
  if (found == 0){
    sprintf(cmd, "%s: Command not found.\n", command);
    return cmd;
  }
  return cmd;
}
