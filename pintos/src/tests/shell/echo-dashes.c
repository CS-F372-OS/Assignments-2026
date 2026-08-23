#include "tests/lib.h"

int
main (void) 
{
  pid_t pid = exec ("shell echo '--help' '-n' '--version'");
  if (pid == -1) {
    fail ("shell exec failed with dash-prefixed arguments");
  }
  
  int status = wait (pid);
  msg ("Shell exited with status %d for dash-prefixed arguments", status);
  
  if (status != 0) {
    fail ("shell should execute echo with dash-prefixed arguments successfully");
  }
  
  msg ("Echo dashes test completed successfully");
  return 0;
}
