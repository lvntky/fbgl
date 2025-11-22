CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -ldl
SDL_FLAGS = $(shell pkg-config --cflags --libs sdl2)

all: libfbgl_preload.so fbgl_viewer

libfbgl_preload.so: fbgl_preload.c fbgl_preload.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ fbgl_preload.c $(LDFLAGS)

fbgl_viewer: fbgl_viewer.c
	$(CC) $(CFLAGS) -o $@ $< $(SDL_FLAGS)

clean:
	rm -f libfbgl_preload.so fbgl_viewer

install: all
	@echo "Built successfully!"
	@echo ""
	@echo "Usage:"
	@echo "  1. Terminal 1: ./fbgl_viewer"
	@echo "  2. Terminal 2: LD_PRELOAD=./libfbgl_preload.so ./your_program"

.PHONY: all clean install
