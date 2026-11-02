# OS Doc - Collaborative Document Editing System

## Assignment Overview

Implement a client-server system called **OS Doc** - a collaborative document editing system. The document consists of 10 lines, with each line containing up to 10 words. Multiple clients on the same node can concurrently read and write words in the document. Only one client can edit a specific word at a time, but multiple clients can read the same word simultaneously when no one is editing it. Note that this assignment is independent of Pintos and any of the previous assignments (yay!)

### Quick Summary

**What**: Build **OS Doc** - a collaborative document with 10 lines and up to 10 words per line
**Who**: Multiple clients can read and write words in the document concurrently
**Goal**: Ensure correct concurrent access and prevent data corruption

**You implement**: `server.c` and `client.c`

**Provided to you**: Makefile, runner.py, test cases, helper functions (`ts_printf`, `print_doc`)

---

## Implementation Constraints

### Allowed IPC and Synchronization Mechanisms

You are **free to use any IPC and synchronization mechanisms** you prefer, with the following constraints:

**Allowed**:
- Shared memory
- Message queues (POSIX or System V)
- Signals
- Mutexes (pthread mutexes)
- Condition variables
- Semaphores
- Pipes (named or unnamed)
- Any other standard POSIX/Unix IPC mechanisms

**Not Allowed**:
- ❌ Sockets (TCP/UDP)
- ❌ Third-party libraries (only standard C library and POSIX APIs)

---

## What You Are Provided

You are given the following files and utilities to help you complete this assignment:

1. **Makefile** - Build system for compiling your C programs
2. **runner.py** - Test orchestration script that:
   - Loads test configurations from JSON files
   - Compiles your code automatically
   - Spawns server and clients
   - Collects and displays output in chronological order
   - Verifies your implementation against expected results
3. **testcases/*.json** - Test configuration files with various scenarios
4. **Helper Functions**:
   - `ts_printf()` - Timestamped printf function that automatically adds timestamps to output
   - `print_doc()` - Function for formatting and writing the final document state to output files

---

## System Components

### 1. Server
- Handles concurrent read and write requests from multiple clients
- Manages access control to ensure only one client can edit a word at a time
- Responds to client requests with success/failure notifications
- Produces the final document state in `output.txt` upon shutdown

### 2. Client
- Reads commands from an input file (`input.txt`)
- Processes only commands designated for its specific client ID
- Sends read and write requests to the server
- Receives and displays acknowledgments from the server
- **Runs a print_doc thread** that:
  - Executes every 2 seconds
  - Issues **special** READ requests to get the updated file, so it can print it.
  - Writes the current document state to `output_client<id>.txt`

### 3. Runner (`runner.py`)
- Runs testcases that are in `testcases/` directory

---

## Document Structure

### OS Doc Format
- **Structure**: 10 lines, each line can contain up to 10 words
- **Total capacity**: 100 words maximum
- **Word format**: Each word is a string with maximum length of **64 characters**
- **Addressing**: Words are identified by position (line, word_position) where both are in range [0-9]
- **Edit behavior**: New edits **overwrite** the existing word at that position

---

## Command Format (`input.txt`)

The input file contains commands for all clients. Each command line follows this format:

```
C<client_id> <COMMAND> <arguments>
```

### Command Types

#### 1. READ
**Format**: `C<client_id> READ <line> <word_pos>`

**Description**: Request to read the word at position (line, word_pos) in the document.

**Example**:
```
C0 READ 3 5
```
Client 0 requests to read the word at line 3, word position 5.

---

#### 2. WRITE
**Format**: `C<client_id> WRITE <line> <word_pos> <string> <time>`

**Description**: Request to write a word to position (line, word_pos) with a specified edit duration.

**Parameters**:
- `<line>`: Line number (0-9)
- `<word_pos>`: Word position in the line (0-9)
- `<string>`: The word to write (max 64 characters, no spaces)
- `<time>`: Duration in milliseconds to hold exclusive edit access

**Example**:
```
C1 WRITE 2 7 HelloWorld 500
```
Client 1 writes "HelloWorld" at line 2, word position 7, and holds exclusive edit access for 500ms.

---

#### 3. SLEEP
**Format**: `C<client_id> SLEEP <time>`

**Description**: Client pauses execution for the specified duration.

**Parameters**:
- `<time>`: Sleep duration in milliseconds

**Example**:
```
C2 SLEEP 1000
```
Client 2 sleeps for 1000 milliseconds (1 second).

---

## Access Control Requirements

### Word-Level Access Control
- Each word in the document can be accessed independently
- Access control is managed on a per-word basis to maximize concurrency

### Write Operations (Editing Words)
1. Only **one client can edit a word at a time**
2. When a client successfully starts editing a word:
   - The word is updated with the new string
   - The client holds exclusive edit access for the specified `<time>` duration (in milliseconds)
   - No other client can read or write that word during this time
3. If a client tries to edit a word that is currently being edited by another client:
   - The write request is **DROPPED**
   - The server notifies the client that the write was dropped
4. After the time duration expires, the word becomes available for other clients

### Read Operations
1. Read operations are **instantaneous** (no duration)
2. **Multiple clients can read the same word simultaneously** - as long as no one is currently editing it
3. If a client tries to read a word while another client is editing it:
   - The read request is **DROPPED**
   - The server notifies the client that the read was dropped
4. If no client is currently editing the word:
   - The read succeeds immediately
   - The server returns the current word to the client

### Key Rules
- **One editor at a time**: Only one client can edit a word at any given moment
- **Editing blocks all access**: While a word is being edited, all other read and write attempts to that word fail
- **Multiple readers allowed**: Multiple clients can read the same word at the same time, but only when no one is editing it
- **All clients get notified**: Every request (READ or WRITE) receives a response indicating success or failure

---

## Client Behavior

### Command Processing
- Each client receives a unique **client ID** (0-9)
- The client reads the entire `input.txt` file
- The client **filters and executes only commands** matching its client ID
- Commands from other clients are ignored
- Commands are executed **sequentially** in the order they appear in the file

### Response Handling
- For each READ or WRITE command, the client must wait for a response from the server
- The client should display the result of each operation (success/dropped/error)

---

## Example Scenario

### Sample `input.txt`:
```
C0 WRITE 0 0 FirstWrite 300
C1 SLEEP 50
C1 READ 0 0
C1 WRITE 0 0 SecondWrite 200
C0 SLEEP 400
C0 READ 0 0
C1 SLEEP 400
C1 WRITE 0 0 ThirdWrite 100
C1 READ 0 0
```

### Expected Behavior:

1. **T=0ms**: Client 0 writes "FirstWrite" to line 0, word 0 and holds exclusive edit access for 300ms
2. **T=50ms**: Client 1 wakes up and attempts to READ line 0, word 0
   - **Result**: READ DROPPED (Client 0 is currently editing this word)
3. **T=50ms**: Client 1 attempts to WRITE to line 0, word 0
   - **Result**: WRITE DROPPED (Client 0 is currently editing this word)
4. **T=300ms**: Client 0's edit duration expires, word becomes available
5. **T=400ms**: Client 0 attempts to READ line 0, word 0
   - **Result**: SUCCESS, reads "FirstWrite"
6. **T=450ms**: Client 1 wakes up and attempts WRITE to line 0, word 0
   - **Result**: SUCCESS, writes "ThirdWrite" and holds exclusive edit access for 100ms
7. **T=550ms**: Client 1's edit duration expires
8. **T=550ms**: Client 1 attempts to READ line 0, word 0
   - **Result**: SUCCESS, reads "ThirdWrite"

---

## Getting Started

### Understanding the Assignment

1. **Read the entire README** to understand the system requirements
2. **Examine the test cases** in `testcases/` directory to see expected behavior
3. **Study the helper functions** (`ts_printf()` and `print_doc()`) provided in the code templates
4. **Review the command format** and access control requirements

### Development Workflow

1. **Implement server.c**:
   - Start with document initialization
   - Implement communication with clients.
   - Add per-word access control mechanism
   - Implement READ and WRITE request handlers
   - Add timestamped logging using `ts_printf()`
   - Add proper cleanup and document output on shutdown

2. **Implement client.c**:
   - Parse commands from input.txt for your client ID
   - Establish communication with server
   - Send READ/WRITE requests and handle responses
   - Implement SLEEP command
   - Implement the `print_doc()` thread to write document state every 2 seconds
   - Use `ts_printf()` for all output logging

3. **Test your implementation**:
   ```bash
   python3 runner.py testcases/test1.json
   python3 runner.py testcases/test2.json
   ```

4. **Debug using timestamps**: The runner.py will display all output in chronological order, making it easy to trace the sequence of events



**Note**: It is not recommended to run server and clients manually, as runner.py handles process management and output collection.

---

## Building and Running

### Manual Compilation (Optional)

If you want to compile manually, use the provided Makefile:

```bash
make
```

This will compile `server.c` and `client.c` and place the executables in the `build/` directory.

To clean build artifacts:

```bash
make clean
```

**Note**: The `runner.py` script automatically compiles the programs using `make`, so manual compilation is not required when running tests.

---

## Running Tests with runner.py

The runner script handles the entire test workflow including compilation, execution, and verification.

**Note**: `runner.py` MUST NOT be modified.

### Test Configuration Format

Tests are defined using JSON files in the `testcases/` directory. Each test file has the following structure:

```json
{
  "test_name": "Test Name",
  "description": "Description of what this test does",
  "num_clients": 2,
  "input": [
    "C0 WRITE 0 0 Hello 100",
    "C0 READ 0 0",
    "C1 WRITE 0 1 World 50",
    "C1 READ 0 1"
  ],
  "expected_output": "Hello World",
  "expected_output_clients": {
    "0": "Hello World",
    "1": "Hello World"
  },
  "expected_logs": {
    "server": [
      "Server: Client 0 WRITE LOCK(0,0) GRANTED",
      "Server: Client 1 WRITE LOCK(0,1) GRANTED"
    ],
    "clients": {
      "0": [
        "Client 0: Requesting WRITE lock for (0,0)",
        "Client 0: WRITE(0,0) = 'Hello', sleeping for 100ms"
      ],
      "1": [
        "Client 1: Requesting WRITE lock for (0,1)",
        "Client 1: WRITE(0,1) = 'World', sleeping for 50ms"
      ]
    }
  }
}
```

#### JSON Fields:
- **`test_name`** (string): Name of the test
- **`description`** (string): Description of the test scenario
- **`num_clients`** (integer): Number of clients to spawn (1-10)
- **`input`** (array of strings): List of commands to write to `input.txt`
- **`expected_output`** (string): Expected content of `output.txt` (server output) after execution
- **`expected_output_clients`** (object, optional): Expected content of `output_client<id>.txt` files
  - Keys are client IDs (as strings: "0", "1", "2", etc.)
  - Values are expected document content from each client's perspective
  - Note: Actual output may contain "???" for words being edited when print_doc runs
- **`expected_logs`** (object, optional): Expected log patterns for verification
  - **`server`** (array): Expected server log patterns
  - **`clients`** (object): Expected log patterns per client
    - Keys are client IDs (as strings: "0", "1", "2", etc.)
    - Values are arrays of expected log patterns for that specific client

### Running a Test

To run a test, use the runner script with a test configuration file:

```bash
python3 runner.py testcases/test1.json
```

### What the Runner Does

1. **Loads Test Configuration**: Reads the JSON file and extracts test parameters
2. **Compiles Programs**: Automatically runs `make` to compile `server.c` and `client.c`
3. **Creates input.txt**: Generates `input.txt` from the `input` array in the JSON
4. **Starts Server**: Launches the server process
5. **Spawns Clients**: Starts the specified number of client processes (based on `num_clients`)
6. **Collects Output**: Captures all stdout from server and clients
7. **Sorts by Timestamp**: Displays output in chronological order based on timestamps
8. **Verifies Server Output**: Compares `output.txt` with `expected_output`
9. **Verifies Client Outputs**: Compares each `output_client<id>.txt` with `expected_output_clients`
10. **Reports Status**: Displays ✓ PASS or ✗ FAIL for each verification
11. **Cleanup**: Removes message queues

### Example Output

```
Test: Single Client Basic Operations
Description: Test basic READ and WRITE operations with a single client

Cleaning up message queues...
Compiling programs using make...
mkdir -p build
gcc -Wall -Wextra -g -c server.c -o build/server.o
gcc -Wall -Wextra -g -o build/server build/server.o -pthread -lrt
Server built successfully
gcc -Wall -Wextra -g -c client.c -o build/client.o
gcc -Wall -Wextra -g -o build/client build/client.o -lrt
Client built successfully
Compilation successful!

Creating input.txt from test configuration...
input.txt created with 6 commands

Starting test with 1 client(s)...

============================================================
COLLECTING OUTPUT (will print in chronological order)...
============================================================

============================================================
EXECUTION LOG (CHRONOLOGICAL ORDER):
============================================================

[SERVER  ] 23:48:14.123 Server: Started. Grid initialized.
[SERVER  ] 23:48:14.234 Server: Created thread for client 0
[CLIENT0 ] 23:48:14.345 Client 0: Starting
...

============================================================
TEST COMPLETED
============================================================

============================================================
SERVER OUTPUT VERIFICATION
============================================================

Expected: Hello World
Actual:   Hello World

✓ SERVER OUTPUT PASSED: Output matches expected result
============================================================

============================================================
CLIENT OUTPUT VERIFICATION
============================================================

--- Verifying output_client0.txt ---
Expected: Hello World
Actual:   Hello World
✓ Client 0 output matches exactly
============================================================
```

### Exit Codes

- **0**: Test passed
- **1**: Test failed


---

## Output Format

The final document state is written to `output.txt` when the server shuts down. The format is:

- Each line of the document is written on a separate line in the file
- Within each line, non-empty words are separated by spaces
- Only lines with data are included (empty lines are skipped)
- Words are written in order (word position 0, 1, 2, ...)

Example with data in line 0 only:
```
Hello World
```

This corresponds to:
- Line 0, Word 0 = "Hello"
- Line 0, Word 1 = "World"
- All other positions are empty

Example with data in multiple lines:
```
I am testing some dropping reads anything after this should
be dropped
```

This corresponds to:
- Line 0: Word 0="I", Word 1="am", Word 2="testing", Word 3="some", Word 4="dropping", Word 5="reads", Word 6="anything", Word 7="after", Word 8="this", Word 9="should"
- Line 1: Word 0="be", Word 1="dropped"
- All other positions are empty

---

## Provided Helper Functions

### ts_printf()

A timestamped printf function that automatically prefixes output with `HH:MM:SS.mmm` format.

**Usage**: Use this function exactly like `printf()` for all output logging.

```c
// Example usage:
ts_printf("Server: Started. Grid initialized.\n");
ts_printf("Client %d: READ(%d,%d) SUCCESS - Value: '%s'\n", client_id, x, y, value);
```

**Implementation**: The function is already implemented in both server.c and client.c templates. It uses `gettimeofday()` to get current time with millisecond precision.

**Important**: All your output statements must use `ts_printf()` instead of regular `printf()` for proper timestamp logging.

### print_doc()

A function for periodically writing the current document state to an output file.

**Purpose**: Continuously monitors and writes the document contents for debugging and verification.

**Requirements**:
- **MUST be called every 2 seconds** in the client, from the start of the process until it exits
- Each client writes to its own file: `output_client<id>.txt` (e.g., `output_client0.txt`)
- To get the current document state, the client must issue **special READ requests**
- These special reads are marked differently from regular reads and are logged separately
- Special reads follow the same access control rules (dropped if word is being edited)
- The only difference is that these requests wont be checked for in the expected logs.
- The Dropped reads from the server side should be logged as:
  ```
  Server: Client <id> PRINT_DOC READ(x,y) DROPPED
  ```

**Format Requirements**:
- Each line of the document is written on a separate line in the file
- Within each line, non-empty words are separated by spaces
- Only lines with data are included (empty lines are skipped)
- Words are written in order (word position 0, 1, 2, ...)


---

## Timestamps

All output from the server and clients includes timestamps in the format:

```
HH:MM:SS.mmm Message
```

Example:
```
23:48:14.555 Server: Client 0 WRITE(0,0) = 'Hello', sleeping for 100ms
```

The runner.py sorts all output chronologically by these timestamps for easier debugging and understanding of concurrent operations.

---

## Testing Requirements

### Single Client Test
- Verify basic READ and WRITE operations
- Confirm correct data storage and retrieval

### Multi-Client Test
- Test concurrent access with 2-10 clients
- Verify concurrent edit attempt scenarios
- Confirm dropped requests are properly handled
- Ensure no race conditions or data corruption

---

## Constraints

- Maximum string length per word: **64 characters**
- Document dimensions: **10×10** (100 words total)
- Maximum clients: **10**
- Edit duration precision: **milliseconds**
- Coordinate ranges: x, y ∈ [0, 9]

---

## Testing Your Implementation

Run the provided test cases to verify your implementation:

```bash
# Test basic single-client operations
python3 runner.py testcases/test1.json

# Test multi-client concurrency
python3 runner.py testcases/test2.json

# Test race conditions and dropped requests
python3 runner.py testcases/test_race_conditions.json

# Test locked words scenario
python3 runner.py testcases/test_locked_words.json
```

The runner will display ✓ PASS or ✗ FAIL for each test, along with detailed logs to help you debug.

## Submission Instructions

You will have to create a separate branch named `ipc` and push your changes to that branch only. The following deliverables should be present in your branch before the deadline:
- A low-level design (LLD) document explaining your implementation strategy and the reasons behind your design choices.
- The modified `server.c`
- The modified `client.c`

DO NOT create/modify other files. 

## Bonus Segment

If you believe that you are a legendary programmer, you can attempt to add more features to the OS Doc on top of your implementation in the `ipc` branch!

Create a separate branch named `ipc_bonus` and build additional features on top of your working implementation. You will have to create another LLD Document (separate from the previous doc) that explains the additional features you have implemented, implementation strategy and the reasoning(s).

### Example Bonus Features

Two directions you could take:

- **Undo and redo** of edits over the document
- **Persistence**, so a server restart does not lose the document

These are only starting points. We would rather see you think of your own features and explain why they are interesting. The only hard rule is that the base assignment must keep working.

### Demo Video

Along with your code and LLD, you must submit a demo video of your additional features:

- The video must be **at most 3 minutes** long
- It must show your additional features actually running, not just slides
- Upload the video to **Google Drive** and keep the shareable link ready. The submission form will be shared later
- Set the link sharing to "Anyone with the link can view" - links that we cannot open will not be evaluated

### Evaluation

- Groups with the best set of additional features will be **shortlisted for a viva**
- The **best group** will receive bonus points in the lab component of this course

**Note**: It is COMPULSORY to complete the non-bonus part of this assignment before you even attempt the bonus segment. Attempting only the bonus segment, i.e adding additional features without having a fully working implementation of the basic OS Doc WILL NOT be considered during evaluations.

Have fun!!!
