// Copyright (c) 2015-2016 Sergio Gonzalez. All rights reserved.
// License: https://github.com/serge-rgb/milton#license


#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE  // To get MAP_ANONYMOUS on linux
#endif
#define __USE_MISC 1  // MAP_ANONYMOUS and MAP_NORESERVE dont' get defined without this
#include <sys/mman.h>
#undef __USE_MISC

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#if 0
#include <X11/extensions/XIproto.h>
#include <X11/keysym.h>
#endif

#elif defined(__MACH__)
#include <sys/mman.h>
#else
#error "This is not the Unix you're looking for"
#endif

#define HEAP_BEGIN_ADDRESS NULL
#if MILTON_DEBUG
#undef HEAP_BEGIN_ADDRESS
#define HEAP_BEGIN_ADDRESS (void*)(1024LL * 1024 * 1024 * 1024)
#endif

#ifndef MAP_ANONYMOUS

#ifndef MAP_ANON
#error "MAP_ANONYMOUS flag not found!"
#else
#define MAP_ANONYMOUS MAP_ANON
#endif

#endif // MAP_ANONYMOUS

#define platform_milton_log printf

#if defined(__MACH__)
// Include header for our SDL hook.
#include "platform_OSX_SDL_hooks.h"
#endif

void milton_fatal(char* message)
{
    milton_log("*** [FATAL] ***: \n\t");
    puts(message);
    exit(EXIT_FAILURE);
}

// TODO: Show a message box, and then die
void milton_die_gracefully(char* message)
{
    milton_log("*** [FATAL] ***: \n\t");
    puts(message);
    exit(EXIT_FAILURE);
}

#ifdef __linux__
void        platform_load_gl_func_pointers() {}
#elif __MACH__
void        platform_load_gl_func_pointers() {}
#endif

typedef struct UnixMemoryHeader_s {
    size_t size;
} UnixMemoryHeader;

void* platform_allocate(size_t size)
{
    u8* ptr = (u8*)mmap(HEAP_BEGIN_ADDRESS, size + sizeof(UnixMemoryHeader),
                   PROT_WRITE | PROT_READ,
                   /*MAP_NORESERVE |*/ MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);
    if (ptr) {
        *((UnixMemoryHeader*)ptr) = (UnixMemoryHeader) {
            .size = size,
        };
        ptr += sizeof(UnixMemoryHeader);
    }
    return ptr;
}

void platform_deallocate_internal(void* ptr)
{
    assert(ptr);
    u8* begin = (u8*)ptr - sizeof(UnixMemoryHeader);
    size_t size = *((size_t*)begin);
    munmap(ptr, size);
}

int main(int argc, char** argv)
{
    milton_main();
}
