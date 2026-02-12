#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#ifdef __linux__

// Forward declarations to avoid including X11 headers in main file
void *linux_open_display();
void linux_close_display(void *display);
void linux_fake_click(void *display);
void *linux_hotkey_listener(void *arg);

#endif

#endif // PLATFORM_LINUX_H
