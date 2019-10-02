#include "sh.h"
#include <signal.h>
#include <stdio.h>


void sig_handler(int signal);

int main( int argc, char **argv, char **envp )
{
  struct sigaction sig = {0};
  sig.sa_handler = SIG_IGN;
  signal(SIGINT, sig_handler);
  sigaction(SIGTERM, &sig, NULL);
  sigaction(SIGTSTP, &sig, NULL);

  return sh(argc, argv, envp);
}

//signal handler
void sig_handler(int signal)
{
  printf("\ncaught signal [%d]\n", signal);
}
