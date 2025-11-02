# FBGL Virtual Framebuffer Emulator

Test your FBGL programs without actual framebuffer hardware using SDL2!

## Quick Start

```bash
# 1. Install dependencies
sudo apt-get install libsdl2-dev build-essential

# 2. Build the emulator
make

# 3. Run your FBGL program
./fbgl_run.sh ./your_fbgl_program
```

## Components

1. **libfbgl_preload.so** - LD_PRELOAD library that intercepts framebuffer calls
2. **fbgl_viewer** - SDL2 window that displays the virtual framebuffer
3. **fbgl_run.sh** - Convenient wrapper script to run everything

## Installation

### Option 1: Using Makefile

```bash
# Download/create these files in the same directory:
# - fbgl_preload.c
# - fbgl_viewer.c
# - fbgl_run.sh
# - Makefile (below)

make all
```

### Option 2: Manual compilation

```bash
# Compile the preload library
gcc -shared -fPIC fbgl_preload.c -ldl -o libfbgl_preload.so

# Compile the viewer
gcc fbgl_viewer.c -lSDL2 -o fbgl_viewer

# Make runner script executable
chmod +x fbgl_run.sh
```

## Usage

### Basic Usage

```bash
./fbgl_run.sh ./my_fbgl_app
```

### With Arguments

```bash
./fbgl_run.sh ./my_fbgl_app --arg1 value1 --arg2 value2
```

### Manual Usage (without script)

```bash
# Terminal 1: Start the viewer
./fbgl_viewer &

# Terminal 2: Run your program with LD_PRELOAD
LD_PRELOAD=./libfbgl_preload.so ./my_fbgl_app
```

## Example Test Program

Create a simple test program (`test_fbgl.c`):

```c
#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <unistd.h>

int main() {
    fbgl_t fb;
    
    if (fbgl_init(NULL, &fb) < 0) {
        return 1;
    }
    
    // Draw some shapes
    for (int i = 0; i < 100; i++) {
        fbgl_set_bg(&fb, FBGL_RGB(0, 0, 0));
        
        // Draw moving circle
        int x = 400 + (int)(200 * cos(i * 0.1));
        int y = 300 + (int)(200 * sin(i * 0.1));
        fbgl_draw_circle_filled(x, y, 50, FBGL_RGB(255, 0, 0), &fb);
        
        // Draw rectangle
        fbgl_point_t tl = {100, 100};
        fbgl_point_t br = {300, 300};
        fbgl_draw_rectangle_outline(tl, br, FBGL_RGB(0, 255, 0), &fb);
        
        usleep(16666); // ~60 FPS
    }
    
    fbgl_destroy(&fb);
    return 0;
}
```

Compile and run:

```bash
gcc test_fbgl.c -o test_fbgl -lm
./fbgl_run.sh ./test_fbgl
```

## Makefile

Create a file named `Makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lSDL2 -ldl -lm

.PHONY: all clean test

all: libfbgl_preload.so fbgl_viewer fbgl_run.sh

libfbgl_preload.so: fbgl_preload.c
	@echo "Building preload library..."
	$(CC) -shared -fPIC $(CFLAGS) $< -ldl -o $@

fbgl_viewer: fbgl_viewer.c
	@echo "Building SDL viewer..."
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

fbgl_run.sh:
	@echo "Making runner script executable..."
	@chmod +x fbgl_run.sh

test: all test_fbgl
	@echo "Running test program..."
	./fbgl_run.sh ./test_fbgl

test_fbgl: test_fbgl.c fbgl.h
	@echo "Compiling test program..."
	$(CC) $(CFLAGS) test_fbgl.c -lm -o test_fbgl

clean:
	@echo "Cleaning up..."
	rm -f libfbgl_preload.so fbgl_viewer test_fbgl
	@# Clean up shared memory
	@ipcrm -M $$(ftok /tmp F 2>/dev/null | awk '{print $$NF}') 2>/dev/null || true

install:
	@echo "Installing to /usr/local/bin..."
	sudo cp libfbgl_preload.so /usr/local/lib/
	sudo cp fbgl_viewer /usr/local/bin/
	sudo cp fbgl_run.sh /usr/local/bin/fbgl-run
	@echo "Done! You can now run: fbgl-run ./your_program"

uninstall:
	@echo "Uninstalling..."
	sudo rm -f /usr/local/lib/libfbgl_preload.so
	sudo rm -f /usr/local/bin/fbgl_viewer
	sudo rm -f /usr/local/bin/fbgl-run
	@echo "Done!"
```

## Troubleshooting

### "Shared memory not found" error

The viewer needs to be started before the FBGL program. Use the `fbgl_run.sh` script which handles this automatically.

### Black screen

Make sure your FBGL program is actually drawing to the framebuffer and not exiting immediately.

### Viewer doesn't start

Check that SDL2 is installed:
```bash
sudo apt-get install libsdl2-dev
```

### Program crashes

Check stderr output for preload messages. Make sure your program is compiled with `-lm` for math functions.

## Configuration

Edit the following constants in the source files to change resolution:

In `fbgl_preload.c` and `fbgl_viewer.c`:
```c
#define VIRTUAL_FB_WIDTH 800
#define VIRTUAL_FB_HEIGHT 600
```

Then recompile:
```bash
make clean && make
```

## How It Works

1. **LD_PRELOAD** intercepts system calls (`open`, `ioctl`, `mmap`, etc.) that your FBGL program makes
2. Instead of real framebuffer device, it provides a shared memory buffer
3. The SDL viewer reads from this shared memory and displays it in a window
4. Your FBGL program thinks it's writing to `/dev/fb0`, but it's actually writing to shared memory

## Performance

The emulator adds minimal overhead:
- Shared memory for zero-copy data transfer
- VSync enabled in SDL for smooth rendering
- ~60 FPS typical performance

## License

This emulator is public domain. Use it however you want!

## Credits

Created for testing FBGL applications without hardware framebuffer access.
