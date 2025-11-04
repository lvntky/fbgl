# Makefile for fbglExamples

# Project settings
PROJECT = fbglExamples
FBGL_HEADER = fbgl.h
EXAMPLES_DIR = examples

# Compiler settings
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -I. -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE
LDFLAGS = -lm

# Find freetype2 using pkg-config
FREETYPE2_CFLAGS = $(shell pkg-config --cflags freetype2)
FREETYPE2_LIBS = $(shell pkg-config --libs freetype2)

# Combine flags
CFLAGS += $(FREETYPE2_CFLAGS)
LDFLAGS += $(FREETYPE2_LIBS)

# Example programs
EXAMPLES = line rectangle red texture framebuf_info text texture_show_fps circle player ray_casting

# Targets
EXAMPLE_BINS = $(EXAMPLES)
RUN_TARGETS = $(addprefix run_, $(EXAMPLES))

# Default target
.PHONY: all
all: $(EXAMPLE_BINS)

# Build each example
$(EXAMPLE_BINS): %: $(EXAMPLES_DIR)/%.c $(FBGL_HEADER)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Run individual examples
.PHONY: $(RUN_TARGETS)
$(RUN_TARGETS): run_%: %
	./$<

# Run all examples
.PHONY: run-examples
run-examples: $(RUN_TARGETS)

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(EXAMPLE_BINS)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all           - Build all examples (default)"
	@echo "  run-examples  - Run all examples"
	@echo "  run_<name>    - Run specific example (e.g., make run_line)"
	@echo "  clean         - Remove built executables"
	@echo ""
	@echo "Examples: $(EXAMPLES)"