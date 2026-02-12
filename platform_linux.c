#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// External reference to the clicking state
extern bool *g_isClicking;
extern pthread_mutex_t g_clickMutex;

void *linux_open_display() { return XOpenDisplay(NULL); }

void linux_close_display(void *display) {
  if (display != NULL) {
    XCloseDisplay((Display *)display);
  }
}

// Click mode: 0 = XTest (universal), 1 = XEvent (focused window only)
static int click_mode = 0; // Default to XTest for compatibility

void linux_fake_click(void *display) {
  if (display == NULL)
    return;

  Display *dpy = (Display *)display;

  if (click_mode == 0) {
    // XTest mode - universal, works everywhere
    XTestFakeButtonEvent(dpy, 1, True, CurrentTime);  // Press
    XTestFakeButtonEvent(dpy, 1, False, CurrentTime); // Release
    XFlush(dpy);
  } else {
    // XEvent mode - safer, only clicks focused window
    // Doesn't interact with gnome-shell, titlebars, gtk applications
    XButtonEvent event;
    memset(&event, 0, sizeof(event));
    event.button = 1; // Left button
    event.same_screen = True;
    event.subwindow = DefaultRootWindow(dpy);

    // Find the actual window under cursor
    while (event.subwindow) {
      event.window = event.subwindow;
      XQueryPointer(dpy, event.window, &event.root, &event.subwindow,
                    &event.x_root, &event.y_root, &event.x, &event.y,
                    &event.state);
    }

    // Send press event
    event.type = ButtonPress;
    XSendEvent(dpy, PointerWindow, True, ButtonPressMask, (XEvent *)&event);
    XFlush(dpy);
    usleep(1000); // 1ms delay between press and release

    // Send release event
    event.type = ButtonRelease;
    XSendEvent(dpy, PointerWindow, True, ButtonReleaseMask, (XEvent *)&event);
    XFlush(dpy);
  }
}

void *linux_hotkey_listener(void *arg) {
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display for hotkey\n");
    return NULL;
  }

  // Initialize XIInput2
  int xi_opcode, event, error;
  if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event,
                       &error)) {
    fprintf(stderr, "X Input extension not available\n");
    XCloseDisplay(display);
    return NULL;
  }

  // Set up event mask for keyboard events
  XIEventMask mask;
  mask.deviceid = XIAllDevices;
  mask.mask_len = XIMaskLen(XI_LASTEVENT);
  mask.mask = calloc(mask.mask_len, sizeof(char));

  XISetMask(mask.mask, XI_KeyPress);
  XISetMask(mask.mask, XI_KeyRelease);

  Window root = DefaultRootWindow(display);
  XISelectEvents(display, root, &mask, 1);
  XSync(display, False);
  free(mask.mask);

  KeyCode f8_keycode = XKeysymToKeycode(display, XK_F8);
  printf("F8 hotkey listener active (XIInput2 events)\n");
  fflush(stdout);

  bool f8_was_pressed = false;

  while (1) {
    XEvent event;
    XNextEvent(display, &event);

    XGenericEventCookie *cookie = &event.xcookie;
    if (XGetEventData(display, cookie) && cookie->type == GenericEvent) {
      XIDeviceEvent *dev_event = cookie->data;

      // Check if it's F8 key
      if (dev_event->detail == f8_keycode) {
        bool f8_is_pressed = (dev_event->evtype == XI_KeyPress);

        // Detect rising edge (key just pressed, not key repeat)
        if (f8_is_pressed && !f8_was_pressed &&
            !(dev_event->flags & XIKeyRepeat)) {
          // Toggle clicking state
          pthread_mutex_lock(&g_clickMutex);
          if (g_isClicking != NULL) {
            *g_isClicking = !(*g_isClicking);
          }
          pthread_mutex_unlock(&g_clickMutex);
        }

        f8_was_pressed = f8_is_pressed;
      }
    }

    XFreeEventData(display, cookie);
  }

  XCloseDisplay(display);
  return NULL;
}

#endif // __linux__
