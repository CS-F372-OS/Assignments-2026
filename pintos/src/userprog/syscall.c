#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdint.h>
#include <stdbool.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "threads/init.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "lib/user/syscall.h"

static void syscall_handler (struct intr_frame *);

/* User memory access functions */
static int get_user (const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byte);
static void check_valid_ptr (const void *ptr);
static void check_valid_buffer (const void *buffer, unsigned size);
static void check_valid_string (const char *str);

/* System call functions */
static void sys_halt (void);
static void sys_exit (int status);
static pid_t sys_exec (const char *cmd_line);
static int sys_wait (pid_t pid);
static bool sys_create (const char *file, unsigned initial_size);
static bool sys_remove (const char *file);
static int sys_open (const char *file);
static int sys_filesize (int fd);
static int sys_read (int fd, void *buffer, unsigned size);
static int sys_write (int fd, const void *buffer, unsigned size);
static void sys_seek (int fd, unsigned position);
static unsigned sys_tell (int fd);
static void sys_close (int fd);

/* File descriptor management */
#define MAX_FDS 128 /* Maximum number of file descriptors per process */

/* File descriptor entry structure */
struct file_descriptor {
    int fd;              /* File descriptor number */
    struct file *file;   /* Pointer to the file */
    struct list_elem elem; /* List element for file descriptor list */
};


/* List of all process control blocks */
static struct list process_list;
static struct lock process_list_lock;

void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
    list_init (&process_list);

    lock_init (&process_list_lock);
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault occurred. */
static int
get_user (const uint8_t *uaddr)
{
    int result;
    asm ("movl $1f, %0; movzbl %1, %0; 1:"
        : "=&a" (result) : "m" (*uaddr));
    return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
    int error_code;
    asm ("movl $1f, %0; movb %b2, %1; 1:"
        : "=&a" (error_code), "=m" (*udst) : "q" (byte));
    return error_code != -1;
}

/* Verifies that the given pointer points to valid user memory.
   Terminates the process if not. */
static void
check_valid_ptr (const void *ptr)
{
    if (ptr == NULL || !is_user_vaddr (ptr) || get_user (ptr) == -1)
        sys_exit (-1);
}

/* Verifies that the given buffer is entirely contained within valid user memory.
   Terminates the process if not. */
static void
check_valid_buffer (const void *buffer, unsigned size)
{
    const uint8_t *buf = buffer;

    /* Check first and last byte of the buffer */
    check_valid_ptr (buf);
    if (size > 0)
        check_valid_ptr (buf + size - 1);
}

/* Verifies that the given string is valid user memory.
   Terminates the process if not. */
static void
check_valid_string (const char *str)
{
    check_valid_ptr (str);

    while (true) {
        int byte = get_user ((const uint8_t *)str);
        if (byte == -1)
            sys_exit (-1);
        if (byte == 0)
            break;
        str++;
    }
}

// /* Reads a 32-bit value at user virtual address UADDR.
//    UADDR must be below PHYS_BASE.
//    Returns the value if successful, exits the process if a segfault occurred. */
// static uint32_t
// get_user_word (const uint32_t *uaddr)
// {
//     check_valid_ptr (uaddr);
//     check_valid_ptr (uaddr + 3);  // Check that the entire word is accessible
//     return *uaddr;
// }

/* Copies a string from user memory into a kernel buffer.
   Terminates the process if the string is invalid. */
static void
copy_in_string (const char *src, char *dst, size_t max_len)
{
    size_t i;

    for (i = 0; i < max_len; i++) {
        int value = get_user ((const uint8_t *)(src + i));
        if (value == -1)
            sys_exit (-1);

        dst[i] = value;
        if (value == 0)
            break;
    }

    /* Ensure null termination */
    if (i == max_len)
        dst[max_len - 1] = '\0';
}

/* Finds a process control block by pid */


/* Get a file descriptor for the current thread */
static int
allocate_fd (struct file *file)
{
    struct thread *cur = thread_current ();
    // threads always have an empty fd_list
    struct file_descriptor *fd_entry;

    /* If this is the first file descriptor, initialize the list */
    if(cur->fd_list == NULL) {
          cur->fd_list = malloc (sizeof (struct list));
          if (cur->fd_list == NULL) {
              free (fd_entry);
              return -1;
          }
          list_init (cur->fd_list);
    }

    fd_entry = malloc (sizeof (struct file_descriptor));
    if (fd_entry == NULL)
        return -1;

    fd_entry->fd = cur->next_fd++;
    fd_entry->file = file;
    list_push_back (cur->fd_list, &fd_entry->elem);

    return fd_entry->fd;
}

/* Find a file descriptor entry */
static struct file_descriptor *
find_fd (int fd)
{
    struct thread *cur = thread_current ();
    struct list_elem *e;
    if ( cur->fd_list == NULL )
        return NULL;


    for (e = list_begin (cur->fd_list); e != list_end (cur->fd_list);
    e = list_next (e)) {
        struct file_descriptor *fd_entry = list_entry (e, struct file_descriptor, elem);
        if (fd_entry->fd == fd)
            return fd_entry;
    }

    return NULL;
}

/* Close and free a file descriptor */
static void
close_fd (int fd)
{
    struct thread *cur = thread_current ();
    struct list_elem *e, *next;

    if (cur->fd_list == NULL)
        return;

    for (e = list_begin (cur->fd_list); e != list_end (cur->fd_list); e = next) {
        next = list_next (e);
        struct file_descriptor *fd_entry = list_entry (e, struct file_descriptor, elem);
        if (fd_entry->fd == fd) {
            file_close (fd_entry->file);
            list_remove (e);
            free (fd_entry);
            break;
        }
    }
}

/* Close all file descriptors */
static void
close_all_fds (void)
{
    struct thread *cur = thread_current ();
    struct list_elem *e, *next;
    if (cur->fd_list == NULL)
        return;


    for (e = list_begin (cur->fd_list); e != list_end (cur->fd_list); e = next) {
        next = list_next (e);
        struct file_descriptor *fd_entry = list_entry (e, struct file_descriptor, elem);
        file_close (fd_entry->file);
        list_remove (e);
        free (fd_entry);
    }

}

static void
syscall_handler (struct intr_frame *f) 
{
    uint32_t *args = ((uint32_t *) f->esp);

    /* Verify that stack pointer is valid */
    check_valid_ptr (args);

    /* Get the system call number */
    int syscall_number = args[0];
    // printf("System call number: %d\n", syscall_number);

    switch (syscall_number)
    {
        case SYS_HALT:                   /* Halt the operating system. */
            sys_halt ();
            break;

        case SYS_EXIT:                   /* Terminate this process. */
            check_valid_ptr (args + 1);
            sys_exit (args[1]);
            break;

        case SYS_EXEC:                   /* Start another process. */
            check_valid_ptr (args + 1);
            check_valid_string ((const char *) args[1]);
            f->eax = sys_exec ((const char *) args[1]);
            break;

        case SYS_WAIT:                   /* Wait for a child process to die. */
            check_valid_ptr (args + 1);
            f->eax = sys_wait (args[1]);
            break;

        case SYS_CREATE:                 /* Create a file. */
            check_valid_ptr (args + 1);
            check_valid_ptr (args + 2);
            check_valid_string ((const char *) args[1]);
            f->eax = sys_create ((const char *) args[1], args[2]);
            break;

        case SYS_REMOVE:                 /* Delete a file. */
            check_valid_ptr (args + 1);
            check_valid_string ((const char *) args[1]);
            f->eax = sys_remove ((const char *) args[1]);
            break;

        case SYS_OPEN:                   /* Open a file. */
            check_valid_ptr (args + 1);
            check_valid_string ((const char *) args[1]);
            f->eax = sys_open ((const char *) args[1]);
            break;

        case SYS_FILESIZE:               /* Obtain a file's size. */
            check_valid_ptr (args + 1);
            f->eax = sys_filesize (args[1]);
            break;

        case SYS_READ:                   /* Read from a file. */
            check_valid_ptr (args + 1);
            check_valid_ptr (args + 2);
            check_valid_ptr (args + 3);
            check_valid_buffer ((void *) args[2], args[3]);
            f->eax = sys_read (args[1], (void *) args[2], args[3]);
            break;

        case SYS_WRITE:                  /* Write to a file. */
            check_valid_ptr (args + 1);
            check_valid_ptr (args + 2);
            check_valid_ptr (args + 3);
            check_valid_buffer ((const void *) args[2], args[3]);
            f->eax = sys_write (args[1], (const void *) args[2], args[3]);
            break;

        case SYS_SEEK:                   /* Change position in a file. */
            check_valid_ptr (args + 1);
            check_valid_ptr (args + 2);
            sys_seek (args[1], args[2]);
            break;

        case SYS_TELL:                   /* Report current position in a file. */
            check_valid_ptr (args + 1);
            f->eax = sys_tell (args[1]);
            break;

        case SYS_CLOSE:                  /* Close a file. */
            check_valid_ptr (args + 1);
            sys_close (args[1]);
            break;

        default:
            printf ("Unimplemented system call: %d\n", syscall_number);
            sys_exit (-1);
            break;
    }
}

/* Halts the system */
static void
sys_halt (void)
{
    shutdown_power_off ();
}

/* Terminates the current user program, returning status to the kernel.
   If the process's parent waits for it, this is the status that will be returned. */
static void
sys_exit (int status)
{
    struct thread *cur = thread_current ();
    pid_t pid = cur->tid;

    // printf ("%s: exit(%d)\n", cur->name, status);

    /* Find process control block and update exit status */
    lock_acquire (&process_list_lock);
    lock_release (&process_list_lock);

    thread_exit ();
}

/* Runs the executable whose name is given in cmd_line, passing any given arguments,
   and returns the new process's program id (pid). */
static pid_t
sys_exec (const char *cmd_line)
{
    char command_copy[128];  /* Limited to 128 bytes, as mentioned in the exercise */
    tid_t child_tid;

    copy_in_string (cmd_line, command_copy, sizeof command_copy);

    /* Create process control block for child process */
    child_tid = process_execute (command_copy);
    if (child_tid == TID_ERROR)
        return -1;

    return child_tid;
}

/* Waits for a child process pid and retrieves the child's exit status. */
static int
sys_wait (pid_t pid)
{
    // TODO: Implement wait syscall
    return process_wait (pid);
}

/* Creates a new file called file initially initial_size bytes in size.
   Returns true if successful, false otherwise. */
static bool
sys_create (const char *file, unsigned initial_size)
{
    char file_copy[128];

    copy_in_string (file, file_copy, sizeof file_copy);

    /* Create the file */
    return filesys_create (file_copy, initial_size);
}

/* Deletes the file called file. Returns true if successful, false otherwise. */
static bool
sys_remove (const char *file)
{
    char file_copy[128];

    copy_in_string (file, file_copy, sizeof file_copy);

    /* Remove the file */
    return filesys_remove (file_copy);
}

/* Opens the file called file. Returns a nonnegative integer handle called a
   "file descriptor" (fd), or -1 if the file could not be opened. */
static int
sys_open (const char *file)
{
    char file_copy[128];
    struct file *file_ptr;
    int fd;

    copy_in_string (file, file_copy, sizeof file_copy);

    /* Open the file */
    file_ptr = filesys_open (file_copy);
    if (file_ptr == NULL)
        return -1;

    /* Allocate a file descriptor */
    fd = allocate_fd (file_ptr);
    if (fd == -1) {
        file_close (file_ptr);
        return -1;
    }

    return fd;
}

/* Returns the size, in bytes, of the file open as fd. */
static int
sys_filesize (int fd)
{
    struct file_descriptor *fd_entry = find_fd (fd);

    if (fd_entry == NULL)
        return -1;

    return file_length (fd_entry->file);
}

/* Reads size bytes from the file open as fd into buffer.
   Returns the number of bytes actually read (0 at end of file),
   or -1 if the file could not be read. */
static int
sys_read (int fd, void *buffer, unsigned size)
{
    struct file_descriptor *fd_entry;
    unsigned i;

    if (fd == STDIN_FILENO) {
        /* Read from the keyboard */
        uint8_t *buf = buffer;
        for (i = 0; i < size; i++) {
            uint8_t c = input_getc ();
            if (!put_user (buf + i, c))
                sys_exit (-1);
        }
        return size;
    }

    /* Read from a file */
    fd_entry = find_fd (fd);
    if (fd_entry == NULL)
        return -1;

    /* Use an intermediate buffer in kernel space for safety */
    uint8_t *kern_buf = malloc (size);
    if (kern_buf == NULL)
        return -1;

    int bytes_read = file_read (fd_entry->file, kern_buf, size);

    if (bytes_read > 0) {
        /* Copy data to user buffer */
        for (i = 0; i < (unsigned) bytes_read; i++) {
            if (!put_user ((uint8_t *)buffer + i, kern_buf[i])) {
                free (kern_buf);
                sys_exit (-1);
            }
        }
    }

    free (kern_buf);
    return bytes_read;
}

/* Writes size bytes from buffer to the file open as fd.
   Returns the number of bytes actually written,
   or -1 if the file could not be written. */
static int
sys_write (int fd, const void *buffer, unsigned size)
{
    // printf("sys_write called with fd: %d, size: %u\n Buf: %s\n", fd, size, buffer);

    struct file_descriptor *fd_entry;

    if (fd == STDOUT_FILENO) {
        /* Write to console */
        const uint8_t *buf = buffer;
        unsigned bytes_to_write = size;

        /* Break up large writes into manageable chunks */
        while (bytes_to_write > 0) {
            unsigned chunk_size = bytes_to_write < 256 ? bytes_to_write : 256;
            char chunk[256];

            /* Copy chunk of data from user memory to kernel memory */
            unsigned i;
            for (i = 0; i < chunk_size; i++) {
                int value = get_user (buf + i);
                if (value == -1)
                    sys_exit (-1);
                chunk[i] = value;
            }

            /* Write the chunk to console */
            putbuf (chunk, chunk_size);

            /* Move to next chunk */
            buf += chunk_size;
            bytes_to_write -= chunk_size;
        }

        return size;
    }

    /* Write to a file */
    fd_entry = find_fd (fd);
    if (fd_entry == NULL)
        return -1;

    /* Use an intermediate buffer in kernel space for safety */
    uint8_t *kern_buf = malloc (size);
    if (kern_buf == NULL)
        return -1;

    /* Copy data from user buffer */
    unsigned i;
    for (i = 0; i < size; i++) {
        int value = get_user ((const uint8_t *)buffer + i);
        if (value == -1) {
            free (kern_buf);
            sys_exit (-1);
        }
        kern_buf[i] = value;
    }

    int bytes_written = file_write (fd_entry->file, kern_buf, size);

    free (kern_buf);
    return bytes_written;
}

/* Changes the next byte to be read or written in open file fd to position,
   expressed in bytes from the beginning of the file. */
static void
sys_seek (int fd, unsigned position)
{
    struct file_descriptor *fd_entry = find_fd (fd);

    if (fd_entry != NULL)
        file_seek (fd_entry->file, position);
}

/* Returns the position of the next byte to be read or written in open file fd,
   expressed in bytes from the beginning of the file. */
static unsigned
sys_tell (int fd)
{
    struct file_descriptor *fd_entry = find_fd (fd);

    if (fd_entry == NULL)
        return 0;

    return file_tell (fd_entry->file);
}

/* Closes file descriptor fd. */
static void
sys_close (int fd)
{
    close_fd (fd);
}
