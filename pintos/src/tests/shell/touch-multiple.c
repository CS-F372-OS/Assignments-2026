#include "tests/lib.h"

int main(void) 
{
    // Create two new files
    pid_t pid = exec("shell touch testfile1.txt testfile2.txt");
    if (pid == -1) fail("shell exec failed for multiple files");
    int status = wait(pid);
    if (status != 0) fail("touch failed to create multiple files");

    msg("Multiple file creation test passed");
    return 0;
}