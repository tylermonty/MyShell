#include <utmpx.h>
#include <stdio.h>
#include "sh.h"

void watchuser()
{
  struct utmpx *up;
  struct pathelement *tmp = user_list;

  setutxent();			/* start at beginning */
  while (up = getutxent())	/* get an entry */
  {
    if ( up->ut_type == USER_PROCESS )	/* only care about users */
    {
      while (tmp){
        if (strcmp(tmp->element, up->ut_user) == 0){
          printf("%s has logged on %s from %s\n", up->ut_user, up->ut_line, up ->ut_host);
          break;
        }
        tmp = tmp->next;
      }
      tmp = user_list;
    }
  }
}
