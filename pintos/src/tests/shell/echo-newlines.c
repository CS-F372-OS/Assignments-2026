#include "tests/lib.h"

int
main (void) 
{
  pid_t pid = exec ("shell echo 'line1\nline2' 'carriage\nreturn'");
  if (pid == -1) {
    fail ("shell exec failed with newline/carriage return arguments");
  }
  
  int status = wait (pid);
  msg ("Shell exited with status %d for newline/carriage return arguments", status);
  
  if (status != 0) {
    fail ("shell should execute echo with newline/carriage return arguments successfully");
  }
  
  msg ("Echo newlines test completed successfully");
  return 0;
}