#include "get_path.h"

#define PROMPTMAX 32
#define MAXARGS 10

struct pathelement *user_list;

int sh( int argc, char **argv, char **envp);
void freeList(struct pathelement* head);
void *watchuserThreadFun(void *vargp);
void *watchmailThreadFun(void *vargp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
int cd(char *directory);
void pwd();
void pid();
void mykill(int pid, int signal);
void prompt(char *prefix);
void printenv(char **env, char *var);
void watchuser();
