# compiler
CFLAGS = -Wall -Wextra -std=c99 -g -Iinclude

# list of  files 
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))
TARGET = bin/main_process

compile: $(TARGET)

# rule: linking object files to target executable
$(TARGET): $(OBJS)
	@mkdir -p bin
	gcc $(CFLAGS) -o $@ $^ 

# rule: compiling source files to object files
build/%.o: src/%.c
	@mkdir -p build
	gcc $(CFLAGS) -c $< -o $@

# rule: cleaning up build artifacts
clean: 
	@rm -rf build bin
	@rm -rf build
	@rm -rf output
	@rm -rf log

run: $(TARGET)
	@./$(TARGET)

.PHONY: compile clean run