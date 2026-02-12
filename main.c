#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

// Platform detection
#ifdef __linux__
#define PLATFORM_LINUX
#include "platform_linux.h"
#include <raylib.h>
#include <unistd.h>
#elif defined(_WIN32)
#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif
#include "platform_windows.h"
#include <raylib.h>
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#include <ApplicationServices/ApplicationServices.h>
#include <raylib.h>
#endif

typedef struct {
  Rectangle rect;
  const char *label;
  bool isHovered;
  bool isPressed;
} Button;

// Draw a button with hover and press effects
void DrawButton(Button *btn, Vector2 mousePos, bool mouseDown) {
  btn->isHovered = CheckCollisionPointRec(mousePos, btn->rect);
  btn->isPressed = btn->isHovered && mouseDown;

  Color btnColor;
  if (btn->isPressed) {
    btnColor = (Color){80, 80, 80, 255}; // Darker when pressed
  } else if (btn->isHovered) {
    btnColor = (Color){140, 140, 140, 255}; // Lighter on hover
  } else {
    btnColor = (Color){110, 110, 110, 255}; // Normal state
  }

  // Draw button background
  DrawRectangleRec(btn->rect, btnColor);
  DrawRectangleLinesEx(btn->rect, 1, BLACK);

  // Draw button label (centered)
  int textWidth = MeasureText(btn->label, 20);
  int textX = btn->rect.x + (btn->rect.width - textWidth) / 2;
  int textY = btn->rect.y + (btn->rect.height - 20) / 2;
  DrawText(btn->label, textX, textY, 20, BLACK);
}

// Check if button was clicked (released on button)
bool IsButtonClicked(Button *btn, Vector2 mousePos, bool mouseReleased) {
  return btn->isHovered && mouseReleased;
}

// Global state for hotkey thread
bool *g_isClicking = NULL;
int g_clickInterval = 200; // Default 200ms

#ifdef PLATFORM_WINDOWS
windows_mutex_t g_clickMutex_win;
void *g_clickMutex = &g_clickMutex_win;
#else
pthread_mutex_t g_clickMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// Perform a mouse click at current cursor position
void performClick() {
#if defined(PLATFORM_LINUX)
  void *display = linux_open_display();
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return;
  }
  linux_fake_click(display);
  linux_close_display(display);
#elif defined(PLATFORM_WINDOWS)
  windows_fake_click();
#elif defined(PLATFORM_MACOS)
  CGEventRef mouseDown = CGEventCreateMouseEvent(
      NULL, kCGEventLeftMouseDown, CGEventGetLocation(CGEventCreate(NULL)),
      kCGMouseButtonLeft);
  CGEventRef mouseUp = CGEventCreateMouseEvent(
      NULL, kCGEventLeftMouseUp, CGEventGetLocation(CGEventCreate(NULL)),
      kCGMouseButtonLeft);
  CGEventPost(kCGHIDEventTap, mouseDown);
  CGEventPost(kCGHIDEventTap, mouseUp);
  CFRelease(mouseDown);
  CFRelease(mouseUp);
#endif
}

// Hotkey listener thread (F8 global hotkey)
void *hotkeyListener(void *arg) {
#if defined(PLATFORM_LINUX)
  return linux_hotkey_listener(arg);
#elif defined(PLATFORM_WINDOWS)
  return windows_hotkey_listener(arg);
#else
  // Placeholder for other platforms
  return NULL;
#endif
}

// Background clicker thread
void *clickerWorker(void *arg) {
  while (1) {
    bool clicking = false;
    int interval = 200;

    // Read shared state safely
#ifdef PLATFORM_WINDOWS
    windows_mutex_lock(&g_clickMutex_win);
    if (g_isClicking)
      clicking = *g_isClicking;
    interval = g_clickInterval;
    windows_mutex_unlock(&g_clickMutex_win);
#else
    pthread_mutex_lock(&g_clickMutex);
    if (g_isClicking)
      clicking = *g_isClicking;
    interval = g_clickInterval;
    pthread_mutex_unlock(&g_clickMutex);
#endif

    if (clicking) {
      performClick();
      // Sleep for the interval
#ifdef PLATFORM_WINDOWS
      windows_sleep(interval);
#else
      usleep(interval * 1000);
#endif
    } else {
      // Sleep briefly to avoid busy waiting
#ifdef PLATFORM_WINDOWS
      windows_sleep(100);
#else
      usleep(100 * 1000);
#endif
    }
  }
  return NULL;
}

int main(void) {
  InitWindow(300, 200, "Idle Clicker");
  SetTargetFPS(60);

  // State variables
  bool isClicking = false;

  // Set up global pointer for hotkey thread
  g_isClicking = &isClicking;

#ifdef PLATFORM_WINDOWS
  // Initialize critical section for Windows
  windows_mutex_init(&g_clickMutex_win);

  // Start hotkey listener thread (Windows)
  windows_thread_t hotkeyThread = windows_thread_create(hotkeyListener, NULL);
  if (hotkeyThread == NULL) {
    fprintf(stderr, "Failed to create hotkey thread\n");
  }

  // Start clicker worker thread (Windows)
  windows_thread_t clickerThread = windows_thread_create(clickerWorker, NULL);
  if (clickerThread == NULL) {
    fprintf(stderr, "Failed to create clicker thread\n");
  }
#else
  // Start hotkey listener thread (POSIX)
  pthread_t hotkeyThread;
  int thread_result = pthread_create(&hotkeyThread, NULL, hotkeyListener, NULL);
  if (thread_result != 0) {
    fprintf(stderr, "Failed to create hotkey thread: %d\n", thread_result);
  }
  pthread_detach(hotkeyThread); // Detach so it cleans up automatically

  // Start clicker worker thread (POSIX)
  pthread_t clickerThread;
  thread_result = pthread_create(&clickerThread, NULL, clickerWorker, NULL);
  if (thread_result != 0) {
    fprintf(stderr, "Failed to create clicker thread: %d\n", thread_result);
  }
  pthread_detach(clickerThread);
#endif

  // Define buttons
  Button minusBtn = {{20, 20, 30, 30}, "-", false, false};
  Button plusBtn = {{250, 20, 30, 30}, "+", false, false};
  Button quitBtn = {{200, 140, 80, 40}, "Quit", false, false};

  // For toggle clicking on/off
  Rectangle statusArea = {20, 140, 100, 40};
  bool statusHovered = false;

  while (!WindowShouldClose()) {
    Vector2 mousePos = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    // Handle minus button
    if (IsButtonClicked(&minusBtn, mousePos, mouseReleased)) {
#ifdef PLATFORM_WINDOWS
      windows_mutex_lock(&g_clickMutex_win);
#else
      pthread_mutex_lock(&g_clickMutex);
#endif
      g_clickInterval -= 10;
      if (g_clickInterval < 50)
        g_clickInterval = 50; // Minimum 50ms
#ifdef PLATFORM_WINDOWS
      windows_mutex_unlock(&g_clickMutex_win);
#else
      pthread_mutex_unlock(&g_clickMutex);
#endif
    }

    // Handle plus button
    if (IsButtonClicked(&plusBtn, mousePos, mouseReleased)) {
#ifdef PLATFORM_WINDOWS
      windows_mutex_lock(&g_clickMutex_win);
#else
      pthread_mutex_lock(&g_clickMutex);
#endif
      g_clickInterval += 10;
      if (g_clickInterval > 2000)
        g_clickInterval = 2000; // Maximum 2000ms
#ifdef PLATFORM_WINDOWS
      windows_mutex_unlock(&g_clickMutex_win);
#else
      pthread_mutex_unlock(&g_clickMutex);
#endif
    }

    // Handle quit button
    if (IsButtonClicked(&quitBtn, mousePos, mouseReleased)) {
      break;
    }

    // Handle status area click (toggle clicking)
    statusHovered = CheckCollisionPointRec(mousePos, statusArea);
    if (statusHovered && mouseReleased) {
#ifdef PLATFORM_WINDOWS
      windows_mutex_lock(&g_clickMutex_win);
#else
      pthread_mutex_lock(&g_clickMutex);
#endif
      isClicking = !isClicking;
#ifdef PLATFORM_WINDOWS
      windows_mutex_unlock(&g_clickMutex_win);
#else
      pthread_mutex_unlock(&g_clickMutex);
#endif
    }

    // Read shared state for drawing
    int currentInterval;
    bool currentClicking;
#ifdef PLATFORM_WINDOWS
    windows_mutex_lock(&g_clickMutex_win);
#else
    pthread_mutex_lock(&g_clickMutex);
#endif
    currentInterval = g_clickInterval;
    currentClicking = isClicking;
#ifdef PLATFORM_WINDOWS
    windows_mutex_unlock(&g_clickMutex_win);
#else
    pthread_mutex_unlock(&g_clickMutex);
#endif

    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Draw first line: minus button, interval label, plus button
    DrawButton(&minusBtn, mousePos, mouseDown);
    DrawButton(&plusBtn, mousePos, mouseDown);

    // Draw interval label (centered)
    char intervalText[32];
    sprintf(intervalText, "%dms", currentInterval);
    int intervalWidth = MeasureText(intervalText, 20);
    DrawText(intervalText, (300 - intervalWidth) / 2, 25, 20, WHITE);

    // Draw hotkey hint
    const char *hotkeyText = "F8 to trigger";
    int hotkeyWidth = MeasureText(hotkeyText, 30);
    DrawText(hotkeyText, (300 - hotkeyWidth) / 2, 85, 30, RED);

    // Draw second line: status label and quit button
    const char *statusText = currentClicking ? "clicking" : "stopped";
    Color statusColor = currentClicking ? GREEN : (Color){120, 120, 120, 255};

    // Draw status area with hover effect
    if (statusHovered) {
      DrawRectangleRec(statusArea, (Color){70, 70, 70, 255});
    }
    DrawText(statusText, 25, 150, 20, statusColor);

    DrawButton(&quitBtn, mousePos, mouseDown);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}