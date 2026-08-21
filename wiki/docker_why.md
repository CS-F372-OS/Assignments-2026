# Why are we using Docker to run Pintos?

## What is Docker?

Docker is a tool that creates a **lightweight, isolated environment** (called a container) that runs on your computer. Think of it as a small, self-contained computer inside your computer.

## Why Docker for Pintos?

Pintos is a toy operating system that needs special tools to compile and run:

1. **Cross-compiler toolchain** - Special programs that can compile code for 32-bit x86 computers (even though you're probably on a 64-bit machine)
2. **QEMU or Bochs** - Programs that simulate an old x86 computer so Pintos can run on it

Installing these tools manually is complicated and different on each operating system. Docker solves this by packaging everything into one image.

## What Happens Inside Docker?

When you run the Docker container:

1. **You get a Ubuntu 18.04 Linux environment** with all the Pintos tools pre-installed
2. **Inside this container, you can:**
   - Navigate to your Pintos code
   - Run `make` to compile Pintos
   - Use `pintos` command to boot Pintos in QEMU
   - Debug and test your code

## What is Mounting?

Mounting is how Docker connects your computer's files to the container:

```bash
docker run -it --rm --name pintos \
  --mount type=bind,source=/path/to/your/pintos,target=/home/PKUOS/pintos \
  pkuflyingpig/pintos bash
```

This command:
- **`source=/path/to/your/pintos`** - Points to your Pintos folder on your computer
- **`target=/home/PKUOS/pintos`** - Where that folder appears inside the container
- **Creates a shared workspace** - Changes you make on your computer appear instantly in the container, and vice versa

## The Workflow

1. **On your computer**: Edit Pintos code with your favorite editor (VS Code, Vim, etc.)
2. **In Docker container**: Compile and run your code
3. **Changes sync automatically** - No need to copy files back and forth

## Why Not Just Install Tools Directly?

- **Different OS requirements** - Windows, Mac, and Linux need different installation steps
- **Version conflicts** - The tools might conflict with other software on your system
- **Complex setup** - Installing cross-compilers manually is error-prone
- **Consistency** - Docker ensures everyone has the exact same environment

## Benefits

- **Same environment everywhere** - Works the same on Windows, Mac, and Linux
- **Clean isolation** - Doesn't mess up your computer
- **Easy reset** - If something breaks, just restart the container
- **Instant file sharing** - Your code and the container are always in sync

Docker lets you focus on **learning Pintos** instead of fighting with tool installation and environment setup.
