#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PROMPTMAX 32

void pwd(){ //prints out current working directory
  char *pwd = getcwd(NULL, PROMPTMAX+1);
  printf("%s\n", pwd);
  free(pwd);
}
