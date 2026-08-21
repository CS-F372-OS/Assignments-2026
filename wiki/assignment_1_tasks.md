# Assignment 1 Tasks

This document contains the main tasks for enhancing the Pintos shell functionality. Each task focuses on implementing robust shell commands with proper argument handling.

## Prerequisites

Before starting these tasks, ensure you have:
- Basic understanding of [shell programming concepts](./introduction_to_shell.md)
- Successfully completed the [environment setup](./assignment_1_environment_setup.md)
- Access to the Pintos development environment

## Git Workflow Setup

**Important:** Follow this Git workflow to avoid merge conflicts and ensure proper submission:

### 1. Update Your Repository
Start by pulling the latest changes from the public repository:
```bash
git pull public main
```

### 2. Create and Switch to Shell Branch
Create a dedicated branch for your shell implementation:
```bash
git checkout -b shell
```

### 3. Development Workflow
- **Always work in the `shell` branch** - never push directly to `main`
- **Make commits regularly** with descriptive messages
- **Push your changes to the `shell` branch** when ready

### 4. Submission Requirements
- **Submissions will only be accepted from the `shell` branch**
- **No submissions from merge branches or main branch**
- **Direct pushes to main can cause merge conflicts** in future updates

### 5. Example Commands
```bash
# After making changes
git add .
git commit -m "Implement robust echo command with argument handling"
git push origin shell

# To continue working later
git checkout shell
git pull origin shell
```

## Task 1: Implement Robust echo Command

**Objective:** Implement a robust echo command that handles various argument types and edge cases gracefully.

### Requirements
The echo command should:
- Handle no arguments gracefully (exit with status 0)
- Process multiple arguments correctly
- Handle arguments with special characters (quotes, backslashes, tabs)
- Process arguments with leading/trailing spaces
- Handle newline and carriage return characters in arguments
- Process dash-prefixed arguments (--help, -n, --version)

### Testing Scenarios
Your implementation should pass these test cases:
- **echo-none**: No arguments - should exit successfully
- **echo-multi**: Multiple arguments, mixed content, many arguments
- **echo-newlines**: Arguments containing newline and carriage return characters
- **echo-dashes**: Arguments starting with dashes
- **touch-multiple**: Touch multiple files in one command

### Expected Behavior
Refer to the `.ck` files in the `tests/shell` folder.

## Task 2: Implement touch Command

**Objective:** Implement a `touch` command that creates empty files.

### Requirements
- Create a new `touch` command that accepts a filename as an argument
- The command should create an empty file if it doesn't exist
- If the file already exists, the command should succeed (no error)
- The command should exit with status 0 on success

### Testing
Your implementation should pass the **touch-create** test:
- Running `touch newfile.txt` should create an empty file
- The shell should report success and exit with status 0
- The created file should exist and be empty (0 bytes)

### Expected Behavior
```
$ touch newfile.txt
newfile.txt: created
touch: exit(0)
$ cat newfile.txt
(empty file)
```

## Implementation Notes

### Source Code Location
The `echo` and `touch` commands are located in the `pintos/src/examples/` folder:
- `echo.c` - Source code for the echo command
- `touch.c` - Source code for the touch command

### Building the Commands
1. **Compile the example commands first:**
   ```bash
   cd pintos/src/examples
   make
   ```
   This will compile `echo` and `touch` executables that will be used by the userprog tests.

2. **Build the userprog kernel:**
   ```bash
   cd ../userprog
   make
   ```

### Rebuilding After Changes
Since this is a compiled language, **you must rebuild after making any changes** to the source code:

1. **After modifying echo.c or touch.c:**
   ```bash
   cd pintos/src/examples
   make
   ```

2. **After modifying shell.c or other userprog files:**
   ```bash
   cd pintos/src/userprog
   make
   ```

### Clean Builds
If you encounter build issues or want to ensure a fresh build:

1. **Clean examples:**
   ```bash
   cd pintos/src/examples
   make clean
   make
   ```

2. **Clean userprog:**
   ```bash
   cd pintos/src/userprog
   make clean
   make
   ```

**Note:** Always run `make` in both directories after making changes to ensure your modifications are compiled and available for testing.

### Testing
For comprehensive testing information, see the [Testing Guide](./testing.md).

1. **Navigate to the build directory:**
   ```bash
   cd pintos/src/userprog/build
   ```

2. **Run all tests:**
   ```bash
   make check
   ```

3. **Run specific shell tests:**
   ```bash
   make tests/shell/echo-none.result
   make tests/shell/echo-multi.result
   make tests/shell/echo-newlines.result
   make tests/shell/echo-dashes.result
   make tests/shell/touch-create.result
   make tests/shell/touch-multiple.result
   ```

**Note:** The testing guide contains additional information about debugging test failures, verbose output, simulator selection, and best practices for test-driven development.

### Shell Robustness
- All commands should handle malformed input gracefully
- Prevent buffer overflows and memory corruption
- Return appropriate exit codes
- Handle extreme input patterns safely