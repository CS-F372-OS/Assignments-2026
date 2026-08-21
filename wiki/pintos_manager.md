# Pintos Manager Script (`pintos_manager.py`)

This Python script helps automate common tasks for working with Pintos in a Docker environment. It provides two main functions:

1. **Building the Docker image** for Pintos development
2. **Starting an interactive Docker session** with your Pintos source code mounted

## Why This Setup?

Curious why we're using Docker instead of installing tools directly on your computer? 

**Read [Why Docker?](./docker_why.md)** to understand:
- What Docker is and how it works
- Why Pintos needs special tools (cross-compiler toolchain, QEMU/Bochs)
- How file mounting creates a seamless development workflow
- Why this approach is better than manual installation

The short answer: Docker gives everyone the **exact same development environment** regardless of your operating system, while keeping your computer clean and avoiding complex toolchain setup.

## Prerequisites

Before using this script, ensure you have the following installed and configured:

1. **Python:** The script can be run with the `python` command.
2. **Docker:** Docker must be installed, running, and accessible by your user.
3. **Dockerfile:** A `Dockerfile` must be present in the same directory as `pintos_manager.py` for the `--build` command.
4. **Repository:** Ensure you have cloned the complete repository which includes the `pintos/` directory.

## Configuration

The script uses these default configurations:
- **Local Pintos Directory:** `./pintos/` (relative to script location, part of this repository)
- **Docker Image Name:** `pintos`
- **Container Mount Point:** `/home/me/pintos`

## Usage

### Basic Syntax
```bash
python pintos_manager.py [COMMAND] [OPTIONS]
```

### Commands

The script requires exactly one of these mutually exclusive commands:

#### `--build`
Builds the Docker image from the local Dockerfile.

```bash
python pintos_manager.py --build
```

**Note (Mac Apple Silicon - M1/M2/M3/M4):** Docker defaults to building ARM images on Apple Silicon. Pintos expects an x86_64 (amd64) environment. Before running the build on Apple Silicon, set the default platform:

```bash
export DOCKER_DEFAULT_PLATFORM=linux/amd64
python pintos_manager.py --build
```

**What it does:**
- Looks for `Dockerfile` in the current directory
- Builds a Docker image tagged as `pintos`
- Does NOT interact with Pintos source code

#### `--start`
Starts an interactive Pintos development session.

```bash
python pintos_manager.py --start
```

**What it does:**
1. Verifies Pintos source code exists in `./pintos/`:
   - Checks that the directory exists and is not empty
   - Exits with helpful error messages if the directory is missing or empty
2. Starts an interactive Docker container with:
   - The `pintos` Docker image (must exist from previous `--build`)
   - Your local `./pintos/` directory mounted to `/home/me/pintos` inside the container
   - A bash shell for interactive development
3. This shell is referred in the remaining of this wiki as the docker shell or the docker container shell.
   - This shell has an environment ( the docker container ) with the necessary packages to build and run pintos

### Options

#### `-v, --verbose`
Enable verbose (DEBUG level) logging output.

```bash
python pintos_manager.py --start --verbose
python pintos_manager.py --build -v
```

### Help
```bash
python pintos_manager.py --help
```

## Typical Workflow

1. **First time setup:**
   ```bash
   # Build the Docker image
   python pintos_manager.py --build
   
   # Start interactive session (verifies Pintos directory exists)
   python pintos_manager.py --start
   ```

## Error Handling

The script includes comprehensive error checking:
- Verifies Docker is installed and accessible
- Checks for required files (Dockerfile for `--build`)
- Verifies Pintos directory exists and is not empty
- Provides informative error messages and logging

## Manual Commands (Alternative to Script)

If you're using `pintos_manager.py`, you don't need this section. These are the exact commands the manager runs under the hood. If you prefer not to use the script, you can run them manually:

### Build Docker Image
```bash
# Equivalent to: python pintos_manager.py --build
docker build . -t pintos
```

Mac (Apple Silicon) users should set the platform before building:

```bash
export DOCKER_DEFAULT_PLATFORM=linux/amd64
docker build . -t pintos
```

### Start Interactive Session
```bash
# Equivalent to: python pintos_manager.py --start

# Ensure you're in the repository root with the pintos/ directory
# Then start interactive Docker container
docker run -it --rm --name pintos --mount type=bind,source="$(pwd)/pintos",target=/home/me/pintos pintos bash
```

### Example with Absolute Path
```bash
# If you need to specify absolute path explicitly
docker run -it --rm --name pintos-interactive --mount type=bind,source="/full/path/to/your/pintos",target=/home/me/pintos pintos bash
```

## Notes

- The script assumes the Docker image name is `pintos` - ensure your Dockerfile creates an appropriate Pintos development environment
- The `--start` command will fail if the Docker image doesn't exist (run `--build` first)
- The `--start` command requires the `./pintos/` directory to exist as part of the cloned repository
- Source code changes are persisted in your local `./pintos/` directory
- Use Ctrl+C to exit the interactive session
- The container is automatically removed when the session ends (`--rm` flag)

