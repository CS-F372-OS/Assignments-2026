#!/usr/bin/env python3

import argparse
import logging
import os
import shutil
import subprocess
import sys

# --- Configuration ---
PINTOS_SUBDIR_NAME = "pintos"  # Name of the subdirectory for Pintos code, relative to CWD
DOCKER_IMAGE_NAME = "pintos" # Tag for the Docker image
DOCKER_MOUNT_TARGET = "/home/me/pintos" # Mount point inside the container

# --- Logger Setup ---
logger = logging.getLogger("PintosManager")
logger.setLevel(logging.DEBUG) # Capture all log levels
# Console Handler
ch = logging.StreamHandler()
ch.setLevel(logging.INFO) # Default console output level
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
logger.addHandler(ch)

# --- Helper Functions ---
def run_command(command_list, cwd=None, shell=False, check=True, capture_output=False, text=True):
    """Helper to run a subprocess command with logging."""
    cmd_str = command_list if shell else " ".join(command_list)
    logger.debug(f"Executing: {cmd_str} (in {cwd or os.getcwd()})")
    try:
        process = subprocess.run(
            command_list,
            cwd=cwd,
            shell=shell,
            check=check,
            capture_output=capture_output,
            text=text
        )
        if capture_output:
            if process.stdout:
                logger.debug(f"Stdout: {process.stdout.strip()}")
            if process.stderr:
                logger.debug(f"Stderr: {process.stderr.strip()}")
        return process
    except subprocess.CalledProcessError as e:
        logger.error(f"Command '{cmd_str}' failed with exit code {e.returncode}.")
        if hasattr(e, 'stdout') and e.stdout:
             logger.error(f"Stdout: {e.stdout.strip()}")
        if hasattr(e, 'stderr') and e.stderr:
             logger.error(f"Stderr: {e.stderr.strip()}")
        raise
    except FileNotFoundError:
        logger.error(f"Command not found: {command_list[0]}. Is it in your PATH?")
        raise

def check_docker_installed():
    """Checks if Docker CLI is available."""
    logger.info("Checking for Docker installation...")
    try:
        run_command(["docker", "--version"], capture_output=True)
        logger.info("Docker is available.")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        logger.error("Docker command not found or not working. Please ensure Docker is installed and in your PATH.")
        return False

def ensure_pintos_directory_exists():
    """
    Ensures the Pintos directory exists at './pintos' subdirectory.
    Since Pintos is now part of the repository, this simply verifies the directory exists.
    Returns the absolute path to the Pintos source directory.
    """
    pintos_dir_path_relative = PINTOS_SUBDIR_NAME
    absolute_pintos_path = os.path.abspath(pintos_dir_path_relative)

    logger.info(f"Checking Pintos directory at '{absolute_pintos_path}' for Docker mount...")

    if not os.path.exists(absolute_pintos_path):
        logger.error(f"Pintos directory not found at '{absolute_pintos_path}'. "
                    f"Please ensure you have cloned the repository correctly and the pintos directory exists.")
        sys.exit(1)
    elif not os.path.isdir(absolute_pintos_path):
        logger.error(f"Path '{absolute_pintos_path}' exists but is not a directory. Please remove or rename it.")
        sys.exit(1)
    elif not os.listdir(absolute_pintos_path):
        logger.error(f"Pintos directory at '{absolute_pintos_path}' is empty. "
                    f"Please ensure you have cloned the repository correctly.")
        sys.exit(1)
    else:
        logger.info(f"Pintos directory found at '{absolute_pintos_path}'.")
    
    return absolute_pintos_path

# --- Main Actions ---
def action_build_docker_image_only():
    """
    Builds the Docker image ('pintos:latest') from ./Dockerfile.
    Does not interact with Pintos source code or run any commands inside the container.
    """
    logger.info(f"Initiating '--build' action: Build Docker image '{DOCKER_IMAGE_NAME}' from ./Dockerfile.")
    dockerfile_dir = os.getcwd() # Assumes Dockerfile is in the current directory
    dockerfile_path = os.path.join(dockerfile_dir, "Dockerfile")

    if not os.path.exists(dockerfile_path):
        logger.error(f"Dockerfile not found in {dockerfile_dir}. Cannot build image.")
        logger.error("Please ensure a Dockerfile is present in the current directory.")
        sys.exit(1)
    try:
        run_command(["docker", "build", ".", "-t", DOCKER_IMAGE_NAME], cwd=dockerfile_dir)
        logger.info(f"Docker image '{DOCKER_IMAGE_NAME}' build command executed. Check Docker output for success/failure.")
    except subprocess.CalledProcessError:
        logger.error(f"Failed to build Docker image '{DOCKER_IMAGE_NAME}'. Check Docker build logs.")
        sys.exit(1)

def check_container_status(container_name):
    """
    Check if a container exists and its status.
    Returns: ('not_found' | 'running' | 'stopped', container_id_or_none)
    """
    try:
        # Check if container exists
        result = run_command(["docker", "ps", "-a", "--filter", f"name={container_name}", "--format", "{{.ID}} {{.Status}}"], 
                           capture_output=True)
        if not result.stdout.strip():
            return 'not_found', None
        
        container_info = result.stdout.strip().split(' ', 1)
        container_id = container_info[0]
        status = container_info[1] if len(container_info) > 1 else ""
        
        if 'Up ' in status:
            return 'running', container_id
        else:
            return 'stopped', container_id
    except subprocess.CalledProcessError:
        return 'not_found', None

def action_start_interactive():
    """
    1. Ensures Pintos directory is available in './pintos/'.
    2. Starts an interactive Pintos session in Docker, mounting the source code.
    3. Handles existing containers by connecting to running ones or restarting stopped ones.
    Assumes Docker image 'pintos:latest' exists (e.g., from a previous '--build').
    """
    logger.info("Initiating '--start' action: Check Pintos directory and start interactive Docker session.")

    # 1. Ensure Pintos directory exists for mounting
    pintos_code_abs_path = ensure_pintos_directory_exists()

    # 2. Check if container already exists
    container_status, container_id = check_container_status("pintos")
    
    if container_status == 'running':
        logger.info("Container 'pintos' is already running. Connecting to existing container...")
        try:
            subprocess.run(["docker", "exec", "-it", "pintos", "bash"])
            logger.info("Disconnected from existing Pintos Docker container.")
        except FileNotFoundError:
            logger.error("Docker command not found. Is Docker installed and in your PATH?")
            sys.exit(1)
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to connect to existing container: {e}")
            sys.exit(1)
        except KeyboardInterrupt:
            logger.info("Session interrupted by user (Ctrl+C).")
        return
    
    elif container_status == 'stopped':
        logger.info("Container 'pintos' exists but is stopped. Removing and recreating...")
        try:
            run_command(["docker", "rm", "pintos"])
            logger.info("Stopped container removed.")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to remove stopped container: {e}")
            sys.exit(1)

    # 3. Start new interactive session
    logger.info(f"Attempting to use Docker image '{DOCKER_IMAGE_NAME}'. "
                f"If it doesn't exist, this command will fail. Use '--build' to create/update it.")

    docker_command_interactive = [
        "docker", "run", "-it", "--rm", # Interactive, TTY, remove on exit
        "--name", "pintos",
        "--mount", f"type=bind,source={pintos_code_abs_path},target={DOCKER_MOUNT_TARGET}",
        DOCKER_IMAGE_NAME, # Relies on this image existing
        "bash" # Start a bash shell
    ]
    logger.info(f"Launching Docker container. You will be dropped into a bash shell.")
    logger.info(f"Your Pintos source is at: {DOCKER_MOUNT_TARGET} inside the container.")
    logger.info(f"Common commands once inside: "
                f"cd {DOCKER_MOUNT_TARGET}/src/threads && make && cd build && pintos -- [args]")
    try:
        subprocess.run(docker_command_interactive)
        logger.info("Interactive Pintos Docker session ended.")
    except FileNotFoundError:
        logger.error("Docker command not found. Is Docker installed and in your PATH?")
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        logger.error(f"Docker run command failed: {e}")
        logger.error(f"Ensure the Docker image '{DOCKER_IMAGE_NAME}' exists. You might need to run with '--build' first.")
        sys.exit(1)
    except KeyboardInterrupt:
        logger.info("Interactive session interrupted by user (Ctrl+C).")

def main():
    parser = argparse.ArgumentParser(
        description=(
            f"Pintos Build and Run Management Script.\n\n"
            f"Global assumptions:\n"
            f"- Dockerfile for '{DOCKER_IMAGE_NAME}' is in the current directory (used by --build).\n"
            f"- Pintos source code (for --start) exists in a subdirectory named './{PINTOS_SUBDIR_NAME}/' as part of this repository."
        ),
        formatter_class=argparse.RawTextHelpFormatter
    )
    action_group = parser.add_mutually_exclusive_group(required=True)
    action_group.add_argument(
        "--build",
        action="store_true",
        help=(
            "ONLY builds the Docker image ('pintos:latest') from the ./Dockerfile.\n"
            "Does NOT interact with Pintos source or run any Pintos-specific commands."
        )
    )
    action_group.add_argument(
        "--start",
        action="store_true",
        help=(
            "Starts an interactive session:\n"
            f"1. Verifies Pintos source code is available in './{PINTOS_SUBDIR_NAME}/' directory.\n"
            f"2. Launches an interactive bash shell in the '{DOCKER_IMAGE_NAME}' Docker container.\n"
            f"   NOTE: This assumes the '{DOCKER_IMAGE_NAME}' image already exists (e.g., from a prior --build)."
        )
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose (DEBUG level) logging to console."
    )

    args = parser.parse_args()

    if args.verbose:
        ch.setLevel(logging.DEBUG)
    logger.debug(f"Script arguments: {args}")

    if not check_docker_installed():
        sys.exit(1)

    if args.build:
        action_build_docker_image_only()
    elif args.start:
        action_start_interactive()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.info("\nOperation cancelled by user (Ctrl+C). Exiting.")
        sys.exit(130)
    except Exception as e:
        logger.error(f"An unexpected error occurred: {e}", exc_info=logger.isEnabledFor(logging.DEBUG))
        sys.exit(1)