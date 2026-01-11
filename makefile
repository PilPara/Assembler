# Makefile for assembler project

# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -ansi -pedantic 

# Define directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = ${BUILD_DIR}/obj
EXE = run-assembler    # Executable directly in current directory

# Source files
# Use the find linux command to find all .c files in the src directory
SRC_FILES = $(shell find $(SRC_DIR) -name '*.c')

# Generate object files from source files 
# The patsubst function is used to map the source files to object files within the build directory
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(EXE)

# Link object files to create the executable
$(EXE): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)

clean-all: clean
	rm -f $(EXE)
	rm -f *.am *.o *.exe *.ent *.ext *.ob

.PHONY: all clean clean-all
