# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS_SERVER = -pthread -lrt
LDFLAGS_CLIENT = -pthread -lrt

# Build directory
BUILD_DIR = build

# Target executables
SERVER = $(BUILD_DIR)/server
CLIENT = $(BUILD_DIR)/client

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Object files
SERVER_OBJ = $(BUILD_DIR)/server.o
CLIENT_OBJ = $(BUILD_DIR)/client.o

# Default target
all: $(BUILD_DIR) $(SERVER) $(CLIENT)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build server
$(SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_SERVER)
	@echo "Server built successfully"

# Build client
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_CLIENT)
	@echo "Client built successfully"

# Compile source files to object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f /dev/mqueue/client_queue_*
	@echo "Cleaned build artifacts and message queues"

# Run with 1 client (default)
run: all
	python3 runner.py 1

# Run with multiple clients (specify NUM_CLIENTS)
run-multi: all
	python3 runner.py $(NUM_CLIENTS)

# Create sample input file
input:
	@if [ ! -f input.txt ]; then \
		echo "Creating sample input.txt..."; \
		echo "C0 WRITE 0 0 Hello 100" > input.txt; \
		echo "C0 READ 0 0" >> input.txt; \
		echo "C0 WRITE 0 1 World 50" >> input.txt; \
		echo "C0 READ 0 1" >> input.txt; \
		echo "C0 SLEEP 200" >> input.txt; \
		echo "C0 READ 0 0" >> input.txt; \
	else \
		echo "input.txt already exists"; \
	fi

# Clean everything including input.txt
distclean: clean
	rm -f input.txt
	rm -rf output_client*.txt
	@echo "Removed input.txt"

# Help target
help:
	@echo "Available targets:"
	@echo "  make            - Build server and client (default)"
	@echo "  make all        - Build server and client"
	@echo "  make server     - Build only server"
	@echo "  make client     - Build only client"
	@echo "  make clean      - Remove executables, objects, and message queues"
	@echo "  make distclean  - Clean everything including input.txt"
	@echo "  make run        - Build and run with 1 client"
	@echo "  make run-multi NUM_CLIENTS=N - Build and run with N clients"
	@echo "  make input      - Create sample input.txt file"
	@echo "  make help       - Show this help message"

# Phony targets
.PHONY: all clean distclean run run-multi input help
