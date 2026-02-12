#ifdef _WIN32

#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

// External reference to the clicking state
extern bool *g_isClicking;
extern void *g_clickMutex; // CRITICAL_SECTION on Windows

void windows_fake_click() {
  INPUT inputs[2] = {0};

  // Mouse down
  inputs[0].type = INPUT_MOUSE;
  inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

  // Mouse up
  inputs[1].type = INPUT_MOUSE;
  inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

  SendInput(2, inputs, sizeof(INPUT));
}

void *windows_hotkey_listener(void *arg) {
  printf("F8 hotkey listener active (GetAsyncKeyState polling)\n");
  fflush(stdout);

  bool f8_was_pressed = false;

  while (1) {
    // Check if F8 is currently pressed
    bool f8_is_pressed = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;

    // Detect rising edge (key just pressed)
    if (f8_is_pressed && !f8_was_pressed) {
      // Toggle clicking state
      EnterCriticalSection((CRITICAL_SECTION *)g_clickMutex);
      if (g_isClicking != NULL) {
        *g_isClicking = !(*g_isClicking);
      }
      LeaveCriticalSection((CRITICAL_SECTION *)g_clickMutex);
    }

    f8_was_pressed = f8_is_pressed;

    // Sleep to avoid busy-waiting (check every 50ms)
    Sleep(50);
  }

  return NULL;
}

// Wrapper functions for Windows threading
void windows_mutex_init(CRITICAL_SECTION *mutex) {
  InitializeCriticalSection(mutex);
}

HANDLE windows_thread_create(void *(*start_routine)(void *), void *arg) {
  return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_routine, arg, 0,
                      NULL);
}

#endif // _WIN32
