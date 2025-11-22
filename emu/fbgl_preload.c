#define _GNU_SOURCE
#include "fbgl_preload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/shm.h>
#include <stdarg.h>

// ============================================================================
// Global State
// ============================================================================

// Framebuffer state (internal to this file)
static fbgl_state_t g_fbgl_state = {
    .virtual_fd = -1,
    .shared_mem = NULL,
    .shm_id = -1,
    .is_initialized = 0,
    .total_writes = 0,
    .bytes_written = 0
};

// Original function pointers (internal to this file)
static fbgl_original_funcs_t g_original = {
    .open = NULL,
    .close = NULL,
    .ioctl = NULL,
    .mmap = NULL,
    .munmap = NULL,
    .write = NULL,
    .read = NULL
};

// ============================================================================
// Internal Helper Functions
// ============================================================================

// Initialize original function pointers
static void fbgl_init_hooks(void) {
    if (!g_original.open) {
        g_original.open = dlsym(RTLD_NEXT, "open");
        g_original.close = dlsym(RTLD_NEXT, "close");
        g_original.ioctl = dlsym(RTLD_NEXT, "ioctl");
        g_original.mmap = dlsym(RTLD_NEXT, "mmap");
        g_original.munmap = dlsym(RTLD_NEXT, "munmap");
        g_original.write = dlsym(RTLD_NEXT, "write");
        g_original.read = dlsym(RTLD_NEXT, "read");
        
        if (!g_original.open || !g_original.close || !g_original.ioctl ||
            !g_original.mmap || !g_original.munmap || !g_original.write) {
            fprintf(stderr, "[FBGL] ERROR: Failed to load original functions\n");
        }
    }
}

// Initialize shared memory for framebuffer emulation
static int fbgl_init_shared_memory(void) {
    if (g_fbgl_state.is_initialized) {
        return 0; // Already initialized
    }
    
    fprintf(stderr, "[FBGL] Attempting to create shared memory: %zu bytes\n", (size_t)FBGL_SIZE);
    
    // First, try to remove any existing segment with this key
    int old_id = shmget(FBGL_SHM_KEY, 0, 0);
    if (old_id >= 0) {
        fprintf(stderr, "[FBGL] Removing old shared memory segment (ID: %d)\n", old_id);
        shmctl(old_id, IPC_RMID, NULL);
    }
    
    // Create shared memory segment (no extra space needed, shmat returns aligned memory)
    g_fbgl_state.shm_id = shmget(FBGL_SHM_KEY, FBGL_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (g_fbgl_state.shm_id < 0) {
        fprintf(stderr, "[FBGL] ERROR: Failed to create shared memory: %s (errno=%d)\n", 
                strerror(errno), errno);
        fprintf(stderr, "[FBGL] Requested size: %zu bytes (%.2f MB)\n", 
                (size_t)FBGL_SIZE, FBGL_SIZE / (1024.0 * 1024.0));
        
        // Try to get system limits
        struct shminfo shm_info;
        if (shmctl(0, IPC_INFO, (struct shmid_ds *)&shm_info) >= 0) {
            fprintf(stderr, "[FBGL] System max shared memory: %lu bytes (%.2f MB)\n",
                    shm_info.shmmax, shm_info.shmmax / (1024.0 * 1024.0));
        }
        return -1;
    }
    
    // Attach shared memory
    g_fbgl_state.shared_mem = shmat(g_fbgl_state.shm_id, NULL, 0);
    if (g_fbgl_state.shared_mem == (void*)-1) {
        fprintf(stderr, "[FBGL] ERROR: Failed to attach shared memory: %s\n", 
                strerror(errno));
        shmctl(g_fbgl_state.shm_id, IPC_RMID, NULL);
        g_fbgl_state.shared_mem = NULL;
        return -1;
    }
    
    fprintf(stderr, "[FBGL] Shared memory attached at: %p\n", g_fbgl_state.shared_mem);
    
    // Clear the framebuffer to black
    memset(g_fbgl_state.shared_mem, 0, FBGL_SIZE);
    
    g_fbgl_state.is_initialized = 1;
    
    fprintf(stderr, "[FBGL] Initialized: %dx%d @ %d bpp, SHM ID: %d, Size: %zu bytes\n", 
            FBGL_WIDTH, FBGL_HEIGHT, FBGL_BPP, g_fbgl_state.shm_id, (size_t)FBGL_SIZE);
    
    return 0;
}

// Check if a path is a framebuffer device
static int fbgl_is_fb_device(const char* pathname) {
    if (!pathname) return 0;
    
    return (strcmp(pathname, "/dev/fb0") == 0 || 
            strcmp(pathname, "/dev/fb") == 0 ||
            strncmp(pathname, "/dev/fb", 7) == 0);
}

// Check if a file descriptor is our virtual framebuffer
static int fbgl_is_virtual_fd(int fd) {
    return (fd == g_fbgl_state.virtual_fd && fd != -1);
}

// Fill fb_var_screeninfo structure
static void fbgl_fill_var_screeninfo(struct fb_var_screeninfo* vinfo) {
    memset(vinfo, 0, sizeof(*vinfo));
    
    vinfo->xres = FBGL_WIDTH;
    vinfo->yres = FBGL_HEIGHT;
    vinfo->xres_virtual = FBGL_WIDTH;
    vinfo->yres_virtual = FBGL_HEIGHT;
    vinfo->bits_per_pixel = FBGL_BPP;
    
    // ARGB8888 format
    vinfo->red.offset = 16;
    vinfo->red.length = 8;
    vinfo->green.offset = 8;
    vinfo->green.length = 8;
    vinfo->blue.offset = 0;
    vinfo->blue.length = 8;
    vinfo->transp.offset = 24;
    vinfo->transp.length = 8;
}

// Fill fb_fix_screeninfo structure
static void fbgl_fill_fix_screeninfo(struct fb_fix_screeninfo* finfo) {
    memset(finfo, 0, sizeof(*finfo));
    
    strncpy(finfo->id, "FBGL_EMU", sizeof(finfo->id) - 1);
    finfo->smem_len = FBGL_SIZE;
    finfo->type = FB_TYPE_PACKED_PIXELS;
    finfo->visual = FB_VISUAL_TRUECOLOR;
    finfo->line_length = FBGL_PITCH;
}

// ============================================================================
// Hooked Functions (Public - must NOT be static!)
// ============================================================================

int open(const char* pathname, int flags, ...) {
    fbgl_init_hooks();
    
    // Check if opening framebuffer device
    if (fbgl_is_fb_device(pathname)) {
        if (fbgl_init_shared_memory() < 0) {
            errno = EIO;
            return -1;
        }
        
        g_fbgl_state.virtual_fd = FBGL_VIRTUAL_FD;
        fprintf(stderr, "[FBGL] open(\"%s\") -> virtual fd %d\n", 
                pathname, g_fbgl_state.virtual_fd);
        return g_fbgl_state.virtual_fd;
    }
    
    // Pass through to original open
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
        return g_original.open(pathname, flags, mode);
    }
    return g_original.open(pathname, flags);
}

int close(int fd) {
    fbgl_init_hooks();
    
    if (fbgl_is_virtual_fd(fd)) {
        fprintf(stderr, "[FBGL] close(fb) - keeping shared memory alive\n");
        g_fbgl_state.virtual_fd = -1;
        return 0;
    }
    
    return g_original.close(fd);
}

int ioctl(int fd, unsigned long request, ...) {
    fbgl_init_hooks();
    
    if (fbgl_is_virtual_fd(fd)) {
        va_list args;
        va_start(args, request);
        void* argp = va_arg(args, void*);
        va_end(args);
        
        fprintf(stderr, "[FBGL] ioctl(fb, 0x%lx)\n", request);
        
        switch (request) {
            case FBIOGET_VSCREENINFO:
                fbgl_fill_var_screeninfo((struct fb_var_screeninfo*)argp);
                return 0;
                
            case FBIOGET_FSCREENINFO:
                fbgl_fill_fix_screeninfo((struct fb_fix_screeninfo*)argp);
                return 0;
                
            case FBIOPUT_VSCREENINFO:
                // Ignore changes to screen info
                fprintf(stderr, "[FBGL] ioctl: FBIOPUT_VSCREENINFO ignored\n");
                return 0;
                
            default:
                fprintf(stderr, "[FBGL] ioctl: Unknown request 0x%lx\n", request);
                return 0;
        }
    }
    
    // Pass through to original ioctl
    va_list args;
    va_start(args, request);
    void* argp = va_arg(args, void*);
    va_end(args);
    return g_original.ioctl(fd, request, argp);
}

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    fbgl_init_hooks();
    
    if (fbgl_is_virtual_fd(fd)) {
        if (!g_fbgl_state.shared_mem) {
            fprintf(stderr, "[FBGL] ERROR: mmap called but shared memory not initialized\n");
            errno = ENOMEM;
            return MAP_FAILED;
        }
        
        fprintf(stderr, "[FBGL] mmap(fb, %zu bytes, offset=%ld) -> %p\n", 
                length, offset, g_fbgl_state.shared_mem);
        return g_fbgl_state.shared_mem;
    }
    
    return g_original.mmap(addr, length, prot, flags, fd, offset);
}

int munmap(void* addr, size_t length) {
    fbgl_init_hooks();
    
    if (addr == g_fbgl_state.shared_mem) {
        fprintf(stderr, "[FBGL] munmap(fb) - keeping shared memory attached\n");
        return 0;
    }
    
    return g_original.munmap(addr, length);
}

ssize_t write(int fd, const void* buf, size_t count) {
    fbgl_init_hooks();
    
    if (fbgl_is_virtual_fd(fd) && g_fbgl_state.shared_mem && buf) {
        size_t to_write = (count < FBGL_SIZE) ? count : FBGL_SIZE;
        memcpy(g_fbgl_state.shared_mem, buf, to_write);
        
        g_fbgl_state.total_writes++;
        g_fbgl_state.bytes_written += to_write;
        
        fprintf(stderr, "[FBGL] write(fb, %zu bytes) - total writes: %zu\n", 
                to_write, g_fbgl_state.total_writes);
        return to_write;
    }
    
    return g_original.write(fd, buf, count);
}

ssize_t read(int fd, void* buf, size_t count) {
    fbgl_init_hooks();
    
    if (fbgl_is_virtual_fd(fd) && g_fbgl_state.shared_mem && buf) {
        size_t to_read = (count < FBGL_SIZE) ? count : FBGL_SIZE;
        memcpy(buf, g_fbgl_state.shared_mem, to_read);
        return to_read;
    }
    
    return g_original.read(fd, buf, count);
}

// ============================================================================
// Constructor/Destructor
// ============================================================================

__attribute__((constructor))
static void fbgl_constructor(void) {
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "FBGL Framebuffer Emulator v1.0\n");
    fprintf(stderr, "Configuration: %dx%d @ %d bpp\n", 
            FBGL_WIDTH, FBGL_HEIGHT, FBGL_BPP);
    fprintf(stderr, "Shared Memory Key: 0x%08X\n", FBGL_SHM_KEY);
    fprintf(stderr, "========================================\n");
}

__attribute__((destructor))
static void fbgl_destructor(void) {
    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, "FBGL Statistics:\n");
    fprintf(stderr, "  Total writes: %zu\n", g_fbgl_state.total_writes);
    fprintf(stderr, "  Bytes written: %zu\n", g_fbgl_state.bytes_written);
    fprintf(stderr, "========================================\n");
    
    if (g_fbgl_state.shared_mem && g_fbgl_state.shared_mem != (void*)-1) {
        shmdt(g_fbgl_state.shared_mem);
        g_fbgl_state.shared_mem = NULL;
    }
}
