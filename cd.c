#include <stdio.h>
#include <string.h>
#include <unistd.h>

int cd(char *directory){ //return an int to tell if chdir worked
  if (chdir(directory) < 0){
    printf("%s: no such file or directory\n", directory);
    return 0;
  }
  return 1;
}
