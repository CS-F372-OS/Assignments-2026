# Setting Up Environment

This guide covers setting up your development environment for the shell assignment. The installation and Docker setup steps are already documented in the [Pintos Manager Script](./pintos_manager.md) guide.

## Installation and Docker Setup

For detailed instructions on installing and setting up the Docker environment, please refer to the [Pintos Manager Script](./pintos_manager.md) documentation.

### Quick Setup Commands

1. **Build the Docker image:**
   ```bash
   python pintos_manager.py --build
   ```

2. **Start the development environment:**
   ```bash
   python pintos_manager.py --start
   ```

For complete details, troubleshooting, and manual alternatives, see the [Pintos Manager Script](./pintos_manager.md) guide.

## Pintos Environment Setup Commands

Once inside the Docker container, follow these commands to set up Pintos for the shell assignment.

### Step 1: Build the userprog directory

The userprog directory contains the user program support that we'll need for the shell assignment.

```bash
cd /home/me/pintos/src/userprog
make
```

This command compiles the user program support, which includes the ability to run user programs and handle file system operations.
- If this step errors out, run step 2 first and then run this step.

### Step 2: Build the examples directory

The examples directory contains sample programs that we'll use for testing.

```bash
cd /home/me/pintos/src/examples
make
```

This builds various example programs including `cat`, `echo`, and `shell` that we'll use in our setup.

### Step 3: Navigate to the build directory

Change to the userprog build directory where we'll create our filesystem.

```bash
cd /home/me/pintos/src/userprog/build
```

### Step 4: Create a filesystem

Create a 5MB filesystem that will store our programs and files.

```bash
pintos-mkdisk filesys.dsk --filesys-size=5
```

This creates a virtual disk file (`filesys.dsk`) with 5MB of space for storing files.

### Step 5: Format the filesystem

Initialize the filesystem so it can be used to store files.

```bash
pintos -- -f -q
```

The `-f` flag formats the filesystem, and `-q` runs the command quietly (minimal output).

### Step 6: Copy programs to the filesystem

Now we'll copy the necessary programs to our filesystem.

**Copy the cat program:**
```bash
pintos -p ../../examples/cat -a cat -- -q
```

**Copy the echo program:**
```bash
pintos -p ../../examples/echo -a echo -- -q
```

**Copy the shell program:**
```bash
pintos -p ../../examples/shell -a shell -- -q
```

**Copy the touch program:**
```bash
pintos -p ../../examples/touch -a touch -- -q
```

These commands copy the programs from the examples directory to the filesystem with specific names.

### Step 7: Verify the setup

Check that all programs were copied correctly by listing the filesystem contents.

```bash
pintos -- -q ls
```

You should see `cat`, `echo`, `shell`and `touch` listed in the output.

### Step 8: Test the shell

Run the shell program to verify everything is working.

```bash
pintos -- run shell
```

This starts the Pintos shell, and you should see a shell prompt where you can type commands.

## Understanding the Commands

### pintos-mkdisk
- Creates a virtual disk file
- `--filesys-size=5` specifies a 5MB filesystem
- This is where your programs and files will be stored

### pintos -- -f -q
- `-f`: Format the filesystem
- `-q`: Quiet mode (minimal output)
- This prepares the filesystem for use

### pintos -p <program> -a <alias> -- -q
- `-p`: Specifies the program path
- `-a`: Gives the program an alias name
- `-- -q`: Runs the command quietly

### pintos -- run <program>
- `run`: Executes a program
- `<program>`: The name of the program to run

## Using the Development Environment

### Starting a New Session

To start a new development session:

```bash
python pintos_manager.py --start
```


### Verification Checklist

After completing the setup, verify that:

- [ ] All build commands completed successfully
- [ ] Filesystem was created and formatted
- [ ] Programs (cat, echo, shell and touch) are copied to filesystem
- [ ] `ls` command shows all programs
- [ ] Shell starts and responds to commands

## Quick Reference

Here's a quick reference for the complete setup process:

```bash
# Build Docker image and start environment (see pintos_manager.md for details)
python pintos_manager.py --build
python pintos_manager.py --start

# Inside Docker container:
cd /home/me/pintos/src/userprog && make
cd /home/me/pintos/src/examples && make
cd /home/me/pintos/src/userprog/build
pintos-mkdisk filesys.dsk --filesys-size=5
pintos -- -f -q
pintos -p ../../examples/cat -a cat -- -q
pintos -p ../../examples/echo -a echo -- -q
pintos -p ../../examples/shell -a shell -- -q
pintos -- -q ls
pintos -- run shell
```

## Next Steps

Once you have successfully set up your environment, you can proceed to:

1. **[Assignment 1 Tasks](./assignment_1_tasks.md)** - Complete the shell enhancement tasks 