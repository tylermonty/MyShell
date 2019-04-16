#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>

void list ( char *dir ) //lists directories and files within dir
{
  DIR *mydir;
  struct dirent *file;
  if ((mydir = opendir(dir)) == NULL){
    printf("list: cannot access ''%s' : no such file or directory ", dir);
  }
  else{
    printf("%s:\n", dir);
    while ((file = readdir(mydir)) != NULL){
      printf("%s ", file->d_name);
    }
    printf("\n");
    free(file);
  }
  free(mydir);
}
