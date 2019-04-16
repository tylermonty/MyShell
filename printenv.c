#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void printenv(char **env, char *var){
  //printf("ARGUMENT: %s\n", var);
  //char *name = getenv("HOME");
  if (var){
    printf("%s\n", getenv(var));
  }
  else{
    int i = 0;
    while (env[i]){
      printf("%s\n", env[i]);
      i++;
    }
  }
}
