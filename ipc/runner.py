#!/usr/bin/env python3
import subprocess
import time
import sys
import os
import re
import json
import glob

def cleanup_message_queues():
    """Clean up any existing message queues"""
    print("Cleaning up message queues...")
    for i in range(10):
        try:
            subprocess.run(['rm', '-f', f'/dev/mqueue/client_queue_{i}'], 
                         stderr=subprocess.DEVNULL)
        except:
            pass

def cleanup_output_files():
    """Remove existing output files (output.txt and output_client*.txt)"""
    print("Cleaning up existing output files...")
    # Remove main output file if present
    try:
        if os.path.exists('output.txt'):
            os.remove('output.txt')
    except Exception as e:
        print(f"Warning: could not remove output.txt: {e}")

    # Remove per-client output files
    try:
        for path in glob.glob('output_client*.txt'):
            try:
                os.remove(path)
            except Exception as e:
                print(f"Warning: could not remove {path}: {e}")
    except Exception as e:
        print(f"Warning: error while globbing output_client files: {e}")

def compile_programs():
    """Compile server.c and client.c using make"""
    print("Compiling programs using make...")
    result = subprocess.run(['make'], capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Compilation failed:\n{result.stderr}")
        return False

    # Print make output
    if result.stdout:
        print(result.stdout)

    print("Compilation successful!\n")
    return True

def load_test_json(json_file):
    """Load test configuration from JSON file"""
    try:
        with open(json_file, 'r') as f:
            test_config = json.load(f)
        return test_config
    except FileNotFoundError:
        print(f"Error: Test file '{json_file}' not found")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in '{json_file}': {e}")
        sys.exit(1)

def create_input_from_json(test_config):
    """Create input.txt from test configuration"""
    print(f"Creating input.txt from test configuration...")
    with open('input.txt', 'w') as f:
        for line in test_config['input']:
            f.write(line + '\n')
    print(f"input.txt created with {len(test_config['input'])} commands\n")

def run_test(test_config, pipe_output=False):
    """Run the server and clients based on test configuration
    
    Args:
        test_config: Test configuration dictionary
        pipe_output: If True, directly pipe output to console without buffering
    """

    # Extract parameters from test config
    num_clients = test_config.get('num_clients', 1)
    test_name = test_config.get('test_name', 'Unnamed Test')
    description = test_config.get('description', '')
    expected_output = test_config.get('expected_output', None)

    print(f"Test: {test_name}")
    if description:
        print(f"Description: {description}")
    print()

    # Cleanup old queues and any previous output files
    cleanup_message_queues()
    cleanup_output_files()

    # Compile programs
    if not compile_programs():
        return False

    # Create input.txt from test config
    create_input_from_json(test_config)
    
    print(f"Starting test with {num_clients} client(s)...\n")
    print("="*60)
    
    # Start server
    print("Starting server...")

    # Always capture output for verification
    server_process = subprocess.Popen(['./build/server'],
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT,
                                     text=True,
                                     bufsize=1)
    
    # Give server time to initialize
    time.sleep(1)
    
    # Start clients
    client_processes = []
    for i in range(num_clients):
        print(f"Starting client {i}...")
        # Always capture output for verification
        client_proc = subprocess.Popen(['./build/client', str(i)],
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.STDOUT,
                                      text=True,
                                      bufsize=1)
        client_processes.append(client_proc)
        time.sleep(0.2)  # Small delay between client starts
    
    # Collect all output with timestamps
    collected_lines = []

    outputs = {
        server_process.stdout: ("SERVER", server_process),
    }
    for i, proc in enumerate(client_processes):
        outputs[proc.stdout] = (f"CLIENT{i}", proc)

    active_processes = set(outputs.values())

    # If pipe mode, print live output
    if pipe_output:
        print("\n" + "="*60)
        print("DIRECT OUTPUT MODE (real-time):")
        print("="*60 + "\n")
    
    if not pipe_output:
        print("\n" + "="*60)
        print("COLLECTING OUTPUT (will print in chronological order)...")
        print("="*60 + "\n")

    while active_processes:
        # Check which processes have output ready
        ready_outputs = []
        for fd in outputs.keys():
            if fd and not fd.closed:
                ready_outputs.append(fd)

        if not ready_outputs:
            break

        for fd in ready_outputs:
            while True:
                line = fd.readline()
                if line:
                    label, proc = outputs[fd]
                    # Collect line with label for later sorting/verification
                    collected_lines.append((label, line.rstrip()))
                    # In pipe mode, print immediately
                    if pipe_output:
                        print(f"[{label:8s}] {line.rstrip()}")
                else:
                    # Process finished
                    label, proc = outputs[fd]
                    if proc.poll() is not None:
                        active_processes.discard((label, proc))
                    break

        # Small delay to prevent busy waiting
        time.sleep(0.01)

        # Check if all clients are done
        all_clients_done = all(proc.poll() is not None for proc in client_processes)
        if all_clients_done:
            shutdown_msg = "\nAll clients finished. Shutting down server..."
            collected_lines.append(("SYSTEM", shutdown_msg))
            if pipe_output:
                print(shutdown_msg)
            time.sleep(1)
            server_process.terminate()
            break

    # Wait for all processes to complete
    for proc in client_processes:
        proc.wait()

    server_process.wait(timeout=2)

    # Parse timestamp from line format: "HH:MM:SS.mmm message"
    def parse_timestamp(line):
        # Match timestamp pattern at start of line
        match = re.match(r'^(\d{2}):(\d{2}):(\d{2})\.(\d{3})\s', line)
        if match:
            h, m, s, ms = map(int, match.groups())
            # Create a sortable tuple (hours, minutes, seconds, milliseconds)
            return (h, m, s, ms)
        return None

    # Separate lines with and without timestamps
    timestamped_lines = []
    non_timestamped_lines = []

    for label, line in collected_lines:
        ts = parse_timestamp(line)
        if ts:
            timestamped_lines.append((ts, label, line))
        else:
            non_timestamped_lines.append((label, line))

    # Sort timestamped lines
    timestamped_lines.sort(key=lambda x: x[0])

    # In non-pipe mode, print sorted chronological output
    if not pipe_output:
        # Sort collected lines by timestamp and print
        print("\n" + "="*60)
        print("EXECUTION LOG (CHRONOLOGICAL ORDER):")
        print("="*60 + "\n")

        # Print sorted timestamped lines
        for ts, label, line in timestamped_lines:
            print(f"[{label:8s}] {line}")

        # Print non-timestamped lines at the end
        if non_timestamped_lines:
            print("\n--- Messages without timestamps ---")
            for label, line in non_timestamped_lines:
                print(f"[{label:8s}] {line}")

    print("\n" + "="*60)
    print("TEST COMPLETED")
    print("="*60)

    # Helper function to strip timestamps from log lines
    def strip_timestamp(line):
        """Remove timestamp (HH:MM:SS.mmm) from start of line"""
        match = re.match(r'^\d{2}:\d{2}:\d{2}\.\d{3}\s+(.*)$', line)
        if match:
            return match.group(1)
        return line

    # Verify logs if expected_logs is provided
    expected_logs = test_config.get('expected_logs', None)
    log_verification_passed = True

    if expected_logs:
        print("\n" + "="*60)
        print("LOG VERIFICATION")
        print("="*60)

        # Collect logs without timestamps for verification
        actual_server_logs = []
        actual_client_logs = []

        for ts, label, line in timestamped_lines:
            stripped_line = strip_timestamp(line)
            if label == "SERVER":
                # Filter out successful PRINT_DOC reads, keep dropped ones
                if "PRINT_DOC READ" in stripped_line and "DROPPED" not in stripped_line:
                    continue  # Ignore successful special reads
                actual_server_logs.append(stripped_line)
            elif label.startswith("CLIENT"):
                actual_client_logs.append(stripped_line)

        # Check server logs if expected
        if 'server' in expected_logs:
            print("\n--- Server Log Verification ---")
            expected_server = expected_logs['server']

            matches = 0
            missing = []
            for expected_line in expected_server:
                found = False
                for actual_line in actual_server_logs:
                    if expected_line in actual_line or actual_line == expected_line:
                        found = True
                        matches += 1
                        break
                if not found:
                    missing.append(expected_line)

            print(f"Expected {len(expected_server)} log patterns")
            print(f"Found {matches} matching patterns")

            if missing:
                print(f"\n✗ Missing {len(missing)} expected server log pattern(s):")
                for line in missing:
                    print(f"  - {line}")
                log_verification_passed = False
            else:
                print("✓ All expected server log patterns found")

        # Check client logs if expected (new format with per-client logs)
        if 'clients' in expected_logs:
            print("\n--- Client Log Verification ---")
            expected_clients = expected_logs['clients']

            # Separate actual logs by client
            client_logs_by_id = {}
            for ts, label, line in timestamped_lines:
                if label.startswith("CLIENT"):
                    stripped_line = strip_timestamp(line)
                    # Extract client ID from label (e.g., "CLIENT0" -> "0")
                    client_id = label.replace("CLIENT", "")
                    if client_id not in client_logs_by_id:
                        client_logs_by_id[client_id] = []
                    client_logs_by_id[client_id].append(stripped_line)

            # Verify each client's logs
            for client_id, expected_client_logs in expected_clients.items():
                print(f"\n  Client {client_id}:")
                actual_logs = client_logs_by_id.get(client_id, [])

                matches = 0
                missing = []
                for expected_line in expected_client_logs:
                    found = False
                    for actual_line in actual_logs:
                        if expected_line in actual_line or actual_line == expected_line:
                            found = True
                            matches += 1
                            break
                    if not found:
                        missing.append(expected_line)

                print(f"    Expected {len(expected_client_logs)} log patterns")
                print(f"    Found {matches} matching patterns")

                if missing:
                    print(f"    ✗ Missing {len(missing)} expected log pattern(s):")
                    for line in missing:
                        print(f"      - {line}")
                    log_verification_passed = False
                else:
                    print(f"    ✓ All expected log patterns found for Client {client_id}")

        # Legacy support: Check old format 'client' (single array for all clients)
        elif 'client' in expected_logs:
            print("\n--- Client Log Verification (Legacy Format) ---")
            expected_client = expected_logs['client']

            matches = 0
            missing = []
            for expected_line in expected_client:
                found = False
                for actual_line in actual_client_logs:
                    if expected_line in actual_line or actual_line == expected_line:
                        found = True
                        matches += 1
                        break
                if not found:
                    missing.append(expected_line)

            print(f"Expected {len(expected_client)} log patterns")
            print(f"Found {matches} matching patterns")

            if missing:
                print(f"\n✗ Missing {len(missing)} expected client log pattern(s):")
                for line in missing:
                    print(f"  - {line}")
                log_verification_passed = False
            else:
                print("✓ All expected client log patterns found")

        print("="*60)

    # Check output if expected_output is provided
    output_verification_passed = True
    if expected_output:
        print("\n" + "="*60)
        print("SERVER OUTPUT VERIFICATION")
        print("="*60)

        if os.path.exists('output.txt'):
            with open('output.txt', 'r') as f:
                actual_output = f.read().strip()

            print(f"\nExpected: {expected_output}")
            print(f"Actual:   {actual_output}")

            if actual_output == expected_output:
                print("\n✓ SERVER OUTPUT PASSED: Output matches expected result")
                output_verification_passed = True
            else:
                print("\n✗ SERVER OUTPUT FAILED: Output does not match expected result")
                output_verification_passed = False
        else:
            print("\n✗ SERVER OUTPUT FAILED: output.txt not found")
            output_verification_passed = False

        print("="*60)

    # Check client output files if expected_output_clients is provided
    client_output_verification_passed = True
    expected_output_clients = test_config.get('expected_output_clients', None)

    if expected_output_clients:
        print("\n" + "="*60)
        print("CLIENT OUTPUT VERIFICATION")
        print("="*60)

        for client_id, expected_client_output in expected_output_clients.items():
            client_file = f'output_client{client_id}.txt'
            print(f"\n--- Verifying {client_file} ---")

            if os.path.exists(client_file):
                with open(client_file, 'r') as f:
                    actual_client_output = f.read().strip()

                print(f"Expected: {expected_client_output}")
                print(f"Actual:   {actual_client_output}")

                # Check if expected pattern matches (ignoring ??? which are transient)
                if actual_client_output == expected_client_output:
                    print(f"✓ Client {client_id} output matches exactly")
                else:
                    # Check if the difference is only due to ???
                    # For now, just do basic matching
                    print(f"✗ Client {client_id} output does not match")
                    client_output_verification_passed = False
            else:
                print(f"✗ {client_file} not found")
                client_output_verification_passed = False

        print("="*60)

    # Overall result
    if expected_logs or expected_output or expected_output_clients:
        result = log_verification_passed and output_verification_passed and client_output_verification_passed
    else:
        result = True

    # Cleanup
    cleanup_message_queues()

    return result

def main():
    """Main function with command line argument support"""

    if len(sys.argv) < 2:
        print("Usage: python3 runner.py <test_json_file> [--pipe]")
        print("  test_json_file: path to test configuration JSON file")
        print("  --pipe: (optional) directly pipe output to console without buffering")
        print("\nExample:")
        print("  python3 runner.py testcases/test1.json")
        print("  python3 runner.py testcases/test1.json --pipe")
        sys.exit(1)

    json_file = sys.argv[1]
    pipe_output = '--pipe' in sys.argv

    try:
        # Load test configuration
        test_config = load_test_json(json_file)

        # Run the test
        result = run_test(test_config, pipe_output=pipe_output)

        if(result): 
            print("\nOverall Test Result: ✓ PASSED" ) 
        else:
            print("\nOverall Test Result: ✗ FAILED" )

        # Exit with appropriate code
        sys.exit(0 if result else 1)

    except KeyboardInterrupt:
        print("\n\nInterrupted by user. Cleaning up...")
        cleanup_message_queues()
        sys.exit(0)

if __name__ == "__main__":
    main()
