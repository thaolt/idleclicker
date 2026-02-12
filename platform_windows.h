#ifndef PLATFORM_WINDOWS_H
#define PLATFORM_WINDOWS_H

#include <stdbool.h>

// Forward declarations for Windows types (to avoid including windows.h in
// main.c)
#ifdef _WIN32
// Opaque wrapper for CRITICAL_SECTION
// CRITICAL_SECTION is 40 bytes on x64 Windows
typedef struct {
  unsigned char opaque[40];
} windows_mutex_t;

// Opaque wrapper for HANDLE
typedef void *windows_thread_t;
#endif

// Function declarations
void windows_fake_click();
void *windows_hotkey_listener(void *arg);

// Wrapper functions for Windows threading (to avoid including windows.h in
// main.c)
void windows_mutex_init(windows_mutex_t *mutex);
windows_thread_t windows_thread_create(void *(*start_routine)(void *),
                                       void *arg);

#endif // PLATFORM_WINDOWS_H
