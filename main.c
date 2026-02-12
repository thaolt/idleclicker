#include <pthread.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

// Platform detection
#ifdef __linux__
#define PLATFORM_LINUX
#include "platform_linux.h"
#elif _WIN32
#include <windows.h>
#define PLATFORM_WINDOWS
#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#define PLATFORM_MACOS
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
pthread_mutex_t g_clickMutex = PTHREAD_MUTEX_INITIALIZER;

// Perform a mouse click at current cursor position
void performClick() {
#ifdef PLATFORM_LINUX
  void *display = linux_open_display();
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return;
  }
  linux_fake_click(display);
  linux_close_display(display);
#elif PLATFORM_WINDOWS
  INPUT inputs[2] = {0};
  inputs[0].type = INPUT_MOUSE;
  inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  inputs[1].type = INPUT_MOUSE;
  inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
  SendInput(2, inputs, sizeof(INPUT));
#elif PLATFORM_MACOS
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
#ifdef PLATFORM_LINUX
  return linux_hotkey_listener(arg);
#endif
  return NULL;
}

int main(void) {
  InitWindow(300, 200, "Idle Clicker");
  SetTargetFPS(60);

  // State variables
  int clickInterval = 200; // Default 200ms
  bool isClicking = false;
  double clickTimer = 0.0;

  // Set up global pointer for hotkey thread
  g_isClicking = &isClicking;

  // Start hotkey listener thread
  pthread_t hotkeyThread;
  int thread_result = pthread_create(&hotkeyThread, NULL, hotkeyListener, NULL);
  if (thread_result != 0) {
    fprintf(stderr, "Failed to create hotkey thread: %d\n", thread_result);
  }
  pthread_detach(hotkeyThread); // Detach so it cleans up automatically

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

    // Update click timer
    if (isClicking) {
      clickTimer += GetFrameTime();
      if (clickTimer >= clickInterval / 1000.0) {
        performClick();
        clickTimer = 0.0;
      }
    }

    // Handle minus button
    if (IsButtonClicked(&minusBtn, mousePos, mouseReleased)) {
      clickInterval -= 10;
      if (clickInterval < 50)
        clickInterval = 50; // Minimum 50ms
    }

    // Handle plus button
    if (IsButtonClicked(&plusBtn, mousePos, mouseReleased)) {
      clickInterval += 10;
      if (clickInterval > 2000)
        clickInterval = 2000; // Maximum 2000ms
    }

    // Handle quit button
    if (IsButtonClicked(&quitBtn, mousePos, mouseReleased)) {
      break;
    }

    // Handle status area click (toggle clicking)
    statusHovered = CheckCollisionPointRec(mousePos, statusArea);
    if (statusHovered && mouseReleased) {
      isClicking = !isClicking;
      clickTimer = 0.0;
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Draw first line: minus button, interval label, plus button
    DrawButton(&minusBtn, mousePos, mouseDown);
    DrawButton(&plusBtn, mousePos, mouseDown);

    // Draw interval label (centered)
    char intervalText[32];
    sprintf(intervalText, "%dms", clickInterval);
    int intervalWidth = MeasureText(intervalText, 20);
    DrawText(intervalText, (300 - intervalWidth) / 2, 25, 20, WHITE);

    // Draw second line: status label and quit button
    const char *statusText = isClicking ? "clicking" : "stopped";
    Color statusColor = isClicking ? GREEN : (Color){120, 120, 120, 255};

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