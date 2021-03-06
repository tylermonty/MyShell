#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <glob.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include "sh.h"

#define BUFFERSIZE 2048
extern char **environ;
pthread_mutex_t lock;
char *cwd, *owd, *homedir;
int background = 0;
char *prmpt;

int sh( int argc, char **argv, char **envp )
{
  prmpt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  //char *cwd, *owd;
  char **args = calloc(MAXARGS+1, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  //char *homedir;
  struct pathelement *pathlist;
  glob_t gbuf;
  noclobber = 0;
  if (pthread_mutex_init(&lock, NULL) != 0){
        printf("\n mutex init has failed\n");
        return 1;
      }

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		// Home directory to start out with

  if ( (cwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(cwd) + 1, sizeof(char));
  memcpy(owd, cwd, strlen(cwd));
  prmpt[0] = ' '; prmpt[1] = '\0';

  while ( go )
  {
    pathlist = get_path(); //put path into linked list

    /* print your prompt */
    printf("%s [%s] ", prmpt, cwd);
    printf(">>> ");

    /* get command line and process */
    char *token;
    if(!(token = fgets(commandline, BUFFERSIZE, stdin))){
      //break;
    }
    if (token == NULL){ //cntrl D
      printf("^D\n");
    }
    else if((strcmp(token, "\n") == 0)){ //enter key
      free(pathlist->element);
      freeList(pathlist);
      continue;
    }
    else{
      size_t len = strlen(commandline);
      if (commandline[len - 1] == '\n') //change /n to /0
        commandline[len - 1] = '\0';

      token = strtok(commandline, " ");
      argsct = 0;
//Wilcards not working, but close
/*
      i = 0;
      while(token != NULL && i < MAXARGS){
          if (strchr(token, '*') != NULL || strchr(token, '?') != NULL){
            int src;
            char **p;
            src = glob(token, 0, NULL, &gbuf);
            if (src == 0){
              for (int cnt = 0; cnt < (MAXARGS - argsct); cnt++){
                len = (int) strlen(*p);
                args[argsct] = gbuf.gl_pathv[cnt];
                strcpy(args[argsct], *p);
                agrsct++;
              }
              globfree(&gbuf);
            }
          }
          else{
            args[argsct]=token;
            argsct++;
          }
          token = strtok(NULL, " ");
        }
      //args[argsct] = (char *)calloc(len, sizeof(char *));
      args[argsct] = NULL;
*/

      while(token != NULL){ //tokenize to put into args
          args[argsct]=token;
          token = strtok(NULL, " ");
          argsct++;
      }
      args[argsct] = NULL;


    //exit
    if (strcmp(commandline, "exit") == 0){  //if exit, exit my shell
      printf("Executing built-in exit\n");
      break;
    }

    else{
      // 0 = no pipe, 1 = pipe stdout->stdin, 2 = pipe stdout&stderr->stdin
      int piped = 0;
      int i;
      int pipe_fd[2];
      char **left_args, **right_args;
      for (i = 0; i < argsct; i++){
        if (strcmp(args[i], "|") == 0){
          piped = 1;
        }else if (strcmp(args[i], "|&") == 0){
          piped = 2;
        }
        break;
      }

      if (piped){
        left_args = calloc(i, sizeof(char*));
        right_args = calloc(argsct-i-1, sizeof(char*));

        for (int j = 0; j < i; j++){
          left_args[j] = args[j];
        }
        for (int j = i+1; j < argsct; j++){
          right_args[j] = args[j];
        }

        pipe(pipe_fd);

        int cid1, cid2;
        if((cid1 = fork()) < 0){
          perror("fork");
        }
        else{
          if(cid1 == 0){//in child process
            if(piped == 2){//include pipe for stderr
              close(STDERR_FILENO);
              dup(pipe_fd[1]);//replace stderr with writing end
            }
            close(STDOUT_FILENO);
            dup(pipe_fd[1]);//replace stdout with writing end
            close(pipe_fd[0]);//close reading end of pipe
            //execute left command with left args
            execute_command(piped, left_args, i, pathlist, environ);
          }else{//in parent process
            if((cid2 = fork()) < 0){
              perror("fork");
            }else{
              if(cid2 == 0){//in right side child process
                close(STDIN_FILENO);
                dup(pipe_fd[0]);//replace stdin with reading end of pipe
                close(pipe_fd[1]);//close writing end of pipe
                //execute right commmand with right-side args
                execute_command(piped, right_args, argsct-i-1, pathlist, environ);
              }else{//in parent process
                int status;
                //close both ends of pipe
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                //wait for later process to finish
                if (waitpid(cid2, &status, 0) > 0) {
                  if (WIFEXITED(status) && (WEXITSTATUS(status) != 0))
                            printf("Exited with %d\n", WEXITSTATUS(status));
                }
              }
            }
          }
        }
      }else{
        execute_command(piped, args, argsct, pathlist, environ);
      }
    }


      int index = 0;
      while (args[index]){ //set args to null
        args[index] = NULL;
        index++;
      }

      free(token);
    }
    free(pathlist->element); //free pathlist
    freeList(pathlist);
  }
  free(args);
  free(prmpt);
  free(commandline);
  free(cwd);
  free(owd);
  free(pathlist->element);
  freeList(pathlist);
  if (user_list){
    struct pathelement* head;
    struct pathelement *tmp;
    head = user_list;
    while (head)
     {
        tmp = head;
        head = head->next;
        free(tmp->element);
        free(tmp);
     }
  }
  pthread_mutex_destroy(&lock);
  //pthread_cancel(thread_id);
  return 0;
}

//function to free the elements in pathlist
void freeList(struct pathelement* head)
{
   struct pathelement* tmp;
   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }
}

void *watchuserThreadFun(void *vargp){
  //printf("print from thread\n");
  while(1){
    pthread_mutex_lock(&lock);
    watchuser();
    pthread_mutex_unlock(&lock);
    sleep(20);
  }
  return NULL;
}

void *watchmailThreadFun(void *vargp){
  //printf("print from thread\n");
  	char * dir = (char *) vargp;
	off_t filesize = 0;
	struct stat stat_buf, new_stat_buf;
	struct timeval time_buf;
	if(stat(dir, &stat_buf) < 0){
		perror("stat error");
		pthread_exit(0);
	}else{
		filesize = stat_buf.st_size;
		while(1){
			//pthread_mutex_lock(&lock);
			if(stat(dir, &stat_buf) < 0){
				perror("stat error");
				pthread_exit(0);
			}else{
				if(filesize < stat_buf.st_size){
					gettimeofday(&time_buf, NULL);
					printf("\a You've Got Mail in %s at %s\n", dir, ctime(&(time_buf.tv_sec)));
				}
				filesize = stat_buf.st_size;
			}
			sleep(1);
		}
	}
  return NULL;
}

void execute_command(int pipe_info, char **args, int argsct, struct pathelement *pathlist, char **environ){
  /* check for each built in command and implement */

    //which
    if (strcmp(args[0], "which") == 0){
      if (argsct == 1){
        fprintf(stderr, "which: Too few arguments.\n");
      }
      else{
        printf("Executing built-in which\n");
        int i = 1;
        while (i < argsct && i < MAXARGS){ //execute for all arguments
          char *cmd = which(args[i], pathlist);
          printf("%s", cmd);
          i++;
          free(cmd);
        }
      }
    }
    //where
    else if (strcmp(args[0], "where") == 0){
      if (argsct == 1){
        fprintf(stderr, "where: Too few arguments.\n");
      }
      else{
        printf("Executing built-in where\n");
        int i = 1;
        while (i < argsct && i < MAXARGS){ //execute for all arguments
          char *cmd = where(args[i], pathlist);
          printf("%s", cmd);
          i++;
          free(cmd);
        }
      }
    }
    //cd
    else if (strcmp(args[0], "cd") == 0){
      if (argsct > 2){
        fprintf(stderr, "cd: Too many arguments.\n");
      }
      else{
        int success = 0;
        printf("Executing built-in cd\n");
        if (argsct == 1){ //no arguments
          success = cd(homedir);
        }
        else if (strcmp(args[1], "-") == 0){ //go to previous directory
          success = cd(owd);
        }
        else{
          success = cd(args[1]);
        }
        if (success){ //change owd and cwd if cd worked
          free(owd);
          owd = cwd;
          cwd = getcwd(NULL, PATH_MAX+1);
        }
      }
    }

    //pwd
    else if (strcmp(args[0], "pwd") == 0){
      printf("Executing built-in pwd\n");
      if (argsct > 1){
        fprintf(stderr, "pwd: ignoring non-option arguments\n");
      }
      pwd();
    }

    //list
    else if (strcmp(args[0], "list") == 0){
      printf("Executing built-in list\n");
      if (argsct == 1){ //no arguments
        list(cwd);
      }
      else{
        int i = 1;
        while (i < argsct && i < MAXARGS){
          list(args[i]);
          printf("\n");
          i++;
        }
      }
    }

    //pid
    else if (strcmp(args[0], "pid") == 0){
      printf("Executing built-in pid\n");
      if (argsct > 1){
        fprintf(stderr, "pid: ignoring non-option arguments.\n");
      }
      pid();
    }

    //kill
    else if (strcmp(args[0], "kill") == 0){
      if (argsct > 3){
        fprintf(stderr, "kill: Too many arguments.\n");
      }
      else if(argsct == 1){
        fprintf(stderr, "kill: Too few arguments.\n");
      }
      else{
        printf("Executing built-in kill\n");
        if (argsct == 2){ //no specified signal
          int pid = atoi(args[1]);
          mykill(pid, 15);
        }
        else{
          int pid = atoi(args[2]);
          int signal = atoi(args[1]);
          signal = signal * -1;
          mykill(pid, signal);
        }
      }
    }

    //prompt
    else if (strcmp(args[0], "prompt") == 0){
      if (argsct > 2){
        fprintf(stderr, "prompt: too many arguments.\n");
      }
      else{
        printf("Executing built-in prompt\n");
        if (argsct == 1){ //no arguments, input prompt
          printf("Input prompt prefix: ");
          if (!fgets(prmpt, BUFFERSIZE, stdin))
            fprintf(stderr, "fgets error\n");
          size_t p_len = strlen(prmpt);
          if (prmpt[p_len - 1] == '\n') //change /n to /0
            prmpt[p_len - 1] = '\0';
        }
        else if (argsct == 2){
          sprintf(prmpt, "%s", args[1]);
        }
      }
    }

    //printenv
    else if(strcmp(args[0], "printenv") == 0){
      if (argsct > 2){
        fprintf(stderr, "printenv: Too many arguments.\n");
      }
      else if (argsct == 1) { //print entire environment
          printenv(environ, NULL);
        }
      else {  // print one ENV variable
        printenv(environ, args[1]);
      }
     }

    //setenv
    else if(strcmp(args[0], "setenv") == 0){
      if (argsct > 3){
        fprintf(stderr, "setenv: Too many arguments.\n");
      }
      else{
        printf("Executing buit-in setenv.\n");
        if (argsct == 1){
          printenv(environ, NULL);
        }
        else if(argsct == 2){ //set args[1] to environment variable as " "
          setenv(args[1], " ", 1);
          if (strcmp(args[1], "HOME") == 0){ //change homedir
            homedir = " ";
          }
        }
        else if (argsct == 3){ //set environment variable args[1] to args[2]
          setenv(args[1], args[2], 1);
          if (strcmp(args[1], "HOME") == 0){ //change homedir
            homedir = args[2];
          }
        }
      }
    }

    //watchuser
    else if (strcmp(args[0], "watchuser") == 0){
      if (argsct > 3){
        fprintf(stderr, "watchuser: Too many arguments.\n");
      }
      else if (argsct == 1){
        fprintf(stderr, "watchuser: Too few arguments.\n");
      }
      else{
        struct pathelement *tmp;
        if (argsct == 2){ //turn on
            if (!user_list){ //add first element
              tmp = calloc(1, sizeof(struct pathelement));
              user_list = tmp;
              tmp->element = calloc(1, (strlen(args[1])+1));
              strncpy(tmp->element, args[1], strlen(args[1]));
              tmp->next = NULL;
              //if (first_run == 0){
              pthread_t thread_id;
                pthread_create(&thread_id, NULL, watchuserThreadFun, NULL);
                //pthread_join(thread_id, NULL);
            //    first_run = 1;
            //  }
            }
            else{ //add next element
              int duplicate = 0;
              struct pathelement *prev;
              tmp = user_list;
              while(tmp){
                if (strcmp(tmp->element, args[1]) == 0){
                  fprintf(stderr, "Already watching user\n");
                  duplicate = 1;
                  break;
                }
                prev = tmp;
                tmp = tmp->next;
              }
              if (!duplicate){
                prev->next = calloc(1, sizeof(struct pathelement));
                prev = prev->next;
                //tmp = calloc(1, sizeof(struct pathelement));
                prev->element = calloc(1, (strlen(args[1])+1));
                strncpy(prev->element, args[1], strlen(args[1]));
                prev->next = NULL;
              }
            /*
            tmp->element = calloc(1, (strlen(args[1])));
            strcpy(tmp->element, args[1]);
            tmp->next = NULL;
            if (first_run == 0){
              pthread_t thread_id;
              pthread_create(&thread_id, NULL, watchuserThreadFun, NULL);
              //pthread_join(thread_id, NULL);
              first_run = 1;
            }
            */
          }
        }
        else{ //turn off
          int found = 0;
          tmp = user_list;
          struct pathelement *prev = NULL;
          while(tmp){
            if (strcmp(tmp->element, args[1]) == 0){
              if (prev){ //if not first element in list, point prev to next element
                prev->next = tmp->next;
              }
              else{
                user_list = tmp->next;
              }
              free(tmp->element);
              free(tmp);
              found = 1;
              break;
            }
            prev = tmp;
            tmp = tmp->next;
          }
          if (!found){
            fprintf(stderr, "not currently watching username\n");
          }

        }
      }

  }

  //watchmail
  else if(strcmp(args[0], "watchmail") == 0){
    if(argsct == 2 && access(args[1], F_OK) == 0){
      ThreadNode *tmp;
      int unique = 1;
      if(mailList){//already threads active
        tmp = mailList;
        if((unique = strcmp(tmp->fileDesc, args[1])) == 0){
          printf("Already watching file: %s\n", tmp->fileDesc);
        }
        while(tmp->next){
          tmp = tmp->next;
          if((unique = strcmp(tmp->fileDesc, args[1])) == 0){
            printf("Already watching file: %s\n", tmp->fileDesc);
          }
        }//add thread to end of list
        if(unique){
          tmp->next = malloc(sizeof(ThreadNode*));
          tmp->next->next = NULL;
          tmp = tmp->next;
        }
      }else{//new list of threads
        mailList = malloc(sizeof(ThreadNode*));
        mailList->next = NULL;
        tmp = mailList;
      }
      if(unique){
        tmp->fileDesc = malloc(strlen(args[1])*sizeof(char));
        strncpy(tmp->fileDesc, args[1], strlen(args[1])+1);
        tmp->fileDesc[strlen(args[1])] = '\0';
        pthread_create(&(tmp->id), NULL, watchmailThreadFun, tmp->fileDesc);
      }
    }else if(argsct == 3 && access(args[1], F_OK) == 0 && strcmp(args[2], "off") == 0){
      ThreadNode *tmp, *prev;//turn thread off
      tmp = mailList;
      while(tmp && strcmp(tmp->fileDesc, args[1]) != 0){
        prev = tmp;//search for matching file description
        tmp = tmp->next;
      }
      if(tmp && strcmp(tmp->fileDesc, args[1]) == 0){//found
        if(tmp == mailList){//head node
          mailList = mailList->next;//move head down
          printf("%s mail watcher off\n", tmp->fileDesc);
          free(tmp->fileDesc);
          pthread_cancel(tmp->id);
          free(tmp);
        }else{
          prev->next = tmp->next;//rearrange pointers
          printf("%s mail watcher off\n", tmp->fileDesc);
          free(tmp->fileDesc);
          pthread_cancel(tmp->id);
          free(tmp);
        }
      }else{
        printf("no mail watcher found for %s\n", args[1]);
      }
    }else if(argsct == 2 || (argsct == 3 && strcmp(args[2], "off") == 0)){
      printf("Could not access file %s\n", args[1]);
    }else if(argsct > 3){
      fprintf(stderr, "watchmail: Too many arguments\n");
    }else if(argsct == 3){
      fprintf(stderr, "watchmail: unrecognized second argument\n");
    }else{
      fprintf(stderr, "watchmail: Too few arguments\n");
    }
  }

  //noclobber
  else if(strcmp(args[0], "noclobber") == 0){
		noclobber = !noclobber;
	}

  else{
          pid_t  pid = 0;
          int status;
          char *cmd = strdup(args[0]);
          glob_t gbuf;
          int i = 1;
          char *cmd_path;

          if (strcmp(args[argsct - 1], "&") == 0){ //check if background process
            background = 1;
            args[argsct - 1] = NULL;
            argsct--;
          }
          if (cmd[0] == '.' || cmd[0] == '/'){ //absolute path
            cmd_path = cmd;
          }
          else{
            cmd_path = which(cmd, pathlist); //find path
          }
          size_t len = strlen(cmd_path);
          if (cmd_path[len - 1] == '\n'){ //change /n to /0
            cmd_path[len - 1] = '\0';
          }
          struct stat buf; //used to tell path is a file
          stat(cmd_path, &buf);
          if (S_ISREG(buf.st_mode) && (access(cmd_path, X_OK) == 0)){
            printf("Executing %s\n", cmd);
          }
          if(!pipe_info)
            pid = fork();
          if (pid == -1){ //error
            printf("can't fork, error occured\n");
            exit(EXIT_FAILURE);
          }
          else if (pid == 0){ //child
      int j, fid, redirected = 0;
      for(j = 0; j < argsct; j++){
       if(j+1 < argsct){
         if(strcmp(args[j], ">") == 0){
           if(!noclobber)
             fid = open(args[j+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
           else
             fid = open(args[j+1], O_WRONLY|O_CREAT, S_IRWXU);
           close(1);
           dup(fid);
           close(fid);
           redirected = 1;
         }else if(strcmp(args[j], ">&") == 0){
           if(!noclobber)
             fid = open(args[j+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
           else
             fid = open(args[j+1], O_WRONLY|O_CREAT, S_IRWXU);
           close(STDOUT_FILENO);
           dup(fid);
           close(STDERR_FILENO);
           dup(fid);
           close(fid);
           redirected = 2;
         }else if(strcmp(args[j], ">>") == 0){
           if(!noclobber)
             fid = open(args[j+1], O_WRONLY|O_CREAT|O_APPEND, S_IRWXU);
           else
             fid = open(args[j+1], O_WRONLY|O_APPEND);
           close(1);
           dup(fid);
           close(fid);
           redirected = 1;
         }else if(strcmp(args[j], ">>&") == 0){
           if(!noclobber)
             fid = open(args[j+1], O_WRONLY|O_CREAT|O_APPEND, S_IRWXU);
           else
             fid = open(args[j+1], O_WRONLY|O_APPEND);
           close(1);
           close(2);
           dup(fid);
           dup(fid);
           close(fid);
           redirected = 2;
         }else if(strcmp(args[j], "<") == 0){
           fid = open(args[j+1], O_RDONLY);
           close(STDIN_FILENO);
           dup(fid);
           close(fid);
           redirected = 3;
         }
         if(redirected){
           for(j; j < argsct; j++)
             args[j] = NULL;
         }
       }
      }
       // printf("child process, pid = %u\n",getpid());
            if (execve(cmd_path, args, environ) < 0){ //exec command
              printf("%s: Command not found.\n", cmd);
              exit(0);
            }
          }
          else{ //parent
            if (background){ //if background process, waitpid with WNOHANG
              if (waitpid(pid, &status, WNOHANG) > 0) {
                 if (WIFEXITED(status) && (WEXITSTATUS(status) != 0))
                   printf("Exited with %d\n", WEXITSTATUS(status));
                 }
                 signal(SIGCHLD, SIG_IGN);
            }
            else{ //not background process
              if (waitpid(pid, &status, 0) > 0) {
         if (WIFEXITED(status) && (WEXITSTATUS(status) != 0))
                   printf("Exited with %d\n", WEXITSTATUS(status));

               }
             }
           }
         free(cmd);
         background = 0;
        }

}
