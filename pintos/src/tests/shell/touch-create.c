/** Tests `touch` creates a new empty file and shell reports success. */

#include "tests/lib.h"
#include <syscall.h>

int
main (void)
{
  /* Run shell to execute touch to create file, then verify file exists
     and is empty. */
  pid_t pid = exec ("shell touch newfile.txt");
  if (pid == -1)
    fail ("shell exec failed for touch-create");

  int status = wait (pid);
  msg ("shell exited %d after touch", status);

  CHECK (status == 0, "shell returned success");

  int fd = open ("newfile.txt");
  CHECK (fd >= 0, "newfile.txt exists");
  CHECK (filesize (fd) == 0, "newfile.txt is empty");
  close (fd);

  return 0;
}