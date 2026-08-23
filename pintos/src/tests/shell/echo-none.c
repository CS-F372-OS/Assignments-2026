/** Tests echo command with no arguments. */

#include "tests/lib.h"

int
main (void) 
{
  pid_t pid = exec ("shell echo");
  if (pid == -1) {
    fail ("shell exec failed with no arguments");
  }
  
  int status = wait (pid);
  msg ("Shell exited with status %d for no arguments", status);
  
  if (status != 0) {
    fail ("shell should execute echo with no arguments successfully");
  }
  
  msg ("Echo no arguments test completed");
  return 0;
}
