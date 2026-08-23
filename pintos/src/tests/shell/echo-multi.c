/** Tests echo command with multiple arguments and complex argument parsing. */

#include "tests/lib.h"
#include <string.h>
#include <stdio.h>

int
main (void) 
{
  // Test 1: Multiple arguments with spaces
  pid_t pid = exec ("shell echo arg1 arg2 arg3 arg4 arg5");
  if (pid == -1) {
    fail ("shell exec failed with multiple arguments");
  }
  
  int status = wait (pid);
  msg ("Shell exited with status %d for multiple arguments", status);
  
  if (status != 0) {
    fail ("shell should execute echo with multiple arguments successfully");
  }
  
  // Test 2: Arguments with mixed content (spaces, tabs, special chars)
  pid = exec ("shell echo 'hello world' 'tab\ttest' 'quote\"test' 'backslash\\test'");
  if (pid == -1) {
    fail ("shell exec failed with mixed content arguments");
  }
  
  status = wait (pid);
  msg ("Shell exited with status %d for mixed content", status);
  
  if (status != 0) {
    fail ("shell should execute echo with mixed content successfully");
  }
  
  // Test 3: Many small arguments to test argument array handling
  char cmd[2048] = "shell echo";
  for (int i = 0; i < 50; i++) {
    char arg[16];
    snprintf(arg, sizeof(arg), " arg%d", i);
    strlcat(cmd, arg, sizeof(cmd));
  }
  
  pid = exec (cmd);
  if (pid == -1) {
    fail ("shell exec failed with many arguments");
  }
  
  status = wait (pid);
  msg ("Shell exited with status %d for many arguments", status);
  
  if (status != 0) {
    fail ("shell should execute echo with many arguments successfully");
  }
  
  msg ("Echo multiple arguments test completed successfully");
  return 0;
}
