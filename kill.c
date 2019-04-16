#include <signal.h>

void mykill(int pid, int signal){ //kills pid with signal
  kill(pid, signal);
}
