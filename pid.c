#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void pid(){ //prints out current process pid
  printf("%d\n", getpid());
}
