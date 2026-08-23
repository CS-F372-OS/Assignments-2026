#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

static void read_line(char line[], size_t size);
static bool backspace(char **pos, char line[]);

int main(int argc, char *argv[]) {
    /* If command provided as argument, execute it directly */
    if (argc > 1) {
        /* Reconstruct the command from argv[1] onwards */
        char command[200] = "";
        for (int i = 1; i < argc; i++) {
            if (i > 1) 
                strlcat(command, " ", sizeof(command));
            strlcat(command, argv[i], sizeof(command));
        }
        
        /* Execute the command */
        if (!strcmp(command, "exit")) {
            return 0;
        }
        else if (!memcmp(command, "cd ", 3)) {
            if (chdir(command + 3) != 0)
                printf("\"%s\": chdir failed\n", command + 3);
        }
        else {
            pid_t pid = exec(command);
            if (pid != -1) {
                int status = wait(pid);
                printf("\"%s\": exit code %d\n", command, status);
                return status;
            }
            else {
                printf("exec failed\n");
                return -1;
            }
        }
        return 0;
    }
    
    /* Interactive mode */
    printf("Shell starting...\n");
    
    for (;;) {
        char command[80];
        
        /* Read command. */
        printf("$ ");
        read_line(command, sizeof command);
        
        /* Execute command. */
        if (!strcmp(command, "exit"))
            break;
        else if (!memcmp(command, "cd ", 3)) {
            if (chdir(command + 3) != 0)
                printf("\"%s\": chdir failed\n", command + 3);
        }
        else if (command[0] == '\0') {
            /* Empty command. */
        }
        else {
            pid_t pid = exec(command);
            if (pid != -1) {
                int status = wait(pid);
                printf("\"%s\": exit code %d\n", command, status);
            }
            else {
                printf("exec failed\n");
            }
        }
    }
    
    printf("Shell exiting.\n");
    return 0;  // Changed from EXIT_SUCCESS to 0
}

/** Reads a line of input from the user into LINE, which has room
   for SIZE bytes.  Handles backspace and Ctrl+U in the ways
   expected by Unix users.  On return, LINE will always be
   null-terminated and will not end in a new-line character. */
static void read_line(char line[], size_t size) {
    char *pos = line;
    
    for (;;) {
        char c;
        if (read(0, &c, 1) <= 0)  // Changed from STDIN_FILENO to 0
            continue;
        
        switch (c) {
        case '\r':
        case '\n':  // Also handle \n in addition to \r
            *pos = '\0';
            putchar('\n');
            return;
            
        case '\b':
        case 127:
            backspace(&pos, line);
            break;
            
        case 21:  // Ctrl+U (ASCII 21)
            while (backspace(&pos, line))
                continue;
            break;
            
        default:
            /* Add character to line. */
            if (pos < line + size - 1) {
                putchar(c);
                *pos++ = c;
            }
            break;
        }
    }
}

/** If *POS is past the beginning of LINE, backs up one character
   position.  Returns true if successful, false if nothing was
   done. */
static bool backspace(char **pos, char line[]) {
    if (*pos > line) {
        /* Back up cursor, overwrite character, back up again. */
        printf("\b \b");
        (*pos)--;
        return true;
    }
    else {
        return false;
    }
}