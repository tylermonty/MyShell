#include "get_path.h"

#define PROMPTMAX 32
#define MAXARGS 10

struct pathelement *user_list;

typedef struct ThreadNode{
	pthread_t id;
	char* fileDesc;
	struct ThreadNode * next;
} ThreadNode;

ThreadNode *mailList;
int noclobber;

int sh( int argc, char **argv, char **envp);
void freeList(struct pathelement* head);
void *watchuserThreadFun(void *vargp);
void *watchmailThreadFun(void *vargp);
void execute_command(int pipe_info, char **args, int argsct, struct pathelement *pathlist, char **environ);
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
