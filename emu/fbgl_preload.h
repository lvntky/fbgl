#ifndef FBGL_PRELOAD_H
#define FBGL_PRELOAD_H

#include <stddef.h>
#include <sys/types.h>

// Framebuffer configuration
#define FBGL_SHM_KEY     0x1234FBCD
#define FBGL_WIDTH       800
#define FBGL_HEIGHT      600
#define FBGL_BPP         32
#define FBGL_BYTES_PP    (FBGL_BPP / 8)
#define FBGL_PITCH       (FBGL_WIDTH * FBGL_BYTES_PP)
#define FBGL_SIZE        (FBGL_PITCH * FBGL_HEIGHT)

// Virtual framebuffer file descriptor magic number
#define FBGL_VIRTUAL_FD  1000

// Framebuffer state structure
typedef struct {
    int virtual_fd;          // Virtual file descriptor for /dev/fb0
    void* shared_mem;        // Pointer to shared memory
    int shm_id;              // Shared memory ID
    int is_initialized;      // Initialization flag
    size_t total_writes;     // Statistics: number of writes
    size_t bytes_written;    // Statistics: total bytes written
} fbgl_state_t;

// Original function pointers structure
typedef struct {
    int (*open)(const char*, int, ...);
    int (*close)(int);
    int (*ioctl)(int, unsigned long, ...);
    void* (*mmap)(void*, size_t, int, int, int, off_t);
    int (*munmap)(void*, size_t);
    ssize_t (*write)(int, const void*, size_t);
    ssize_t (*read)(int, void*, size_t);
} fbgl_original_funcs_t;


#endif // FBGL_PRELOAD_H
