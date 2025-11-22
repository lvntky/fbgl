# FBGL Framebuffer Emulator

Professional LD_PRELOAD-based framebuffer emulator with clean architecture.

## Structure

- `fbgl_preload.h` - Public header with configuration and types
- `fbgl_preload.c` - Implementation with hook functions
- `sdl_viewer.c` - SDL-based viewer application

## Features

- Clean struct-based state management
- Proper header/source separation
- Statistics tracking
- Error handling
- FPS counter in viewer

## Building

```bash
make
```

Or manually:
```bash
gcc -shared -fPIC -o libfbgl_preload.so fbgl_preload.c -ldl
gcc -o sdl_viewer sdl_viewer.c $(pkg-config --cflags --libs sdl2)
```

## Usage


Terminal 2:
```bash
LD_PRELOAD=./libfbgl_preload.so ./your_fbgl_program
```

Terminal 1:
```bash
./fbgl_viewer
```


## Architecture

1. Hook library intercepts framebuffer syscalls
2. Redirects to shared memory segment
3. SDL viewer displays shared memory contents
4. Program thinks it's using real /dev/fb0

## Configuration

Edit `fbgl_preload.h` to change:
- Screen resolution (FBGL_WIDTH, FBGL_HEIGHT)
- Color depth (FBGL_BPP)
- Shared memory key (FBGL_SHM_KEY)
