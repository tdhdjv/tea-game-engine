#include "platform.h"
#include "../core/logger.h"
#include "../core/input.h"

#if TPLATFORM_WINDOWS

#include <windows.h>
#include <windowsx.h>

typedef struct PlatformState {
  HINSTANCE hInstance;
  HWND handleWindow;
} PlatformState;

//Clock
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(void** platformState, const char* applicationName, i32 x, i32 y, i32 width, i32 height) {
  *platformState = malloc(sizeof(PlatformState));
  PlatformState *state = (PlatformState*)*platformState; 

  state->hInstance = GetModuleHandleA(0);

  //windowing
  HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);
  WNDCLASSA wc;
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = win32_process_message;
  wc.cbWndExtra = 0;
  wc.hInstance = state->hInstance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // NULL; // Manage the cursor manually
  wc.hbrBackground = NULL;                   // Transparent
  wc.lpszClassName = "tea_window_class";

  if(!RegisterClassA(&wc)) {
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }

  //create window
  u32 clientX= x;
  u32 clientY = y;
  u32 clientWidth = width;
  u32 clientHeight = height;

  u32 windowX = clientX;
  u32 windowY = clientY;
  u32 windowWidth = clientWidth;
  u32 windowHeight = clientHeight;

  u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 windowExStyle = WS_EX_APPWINDOW;

  windowStyle |= WS_MAXIMIZEBOX;
  windowStyle |= WS_MINIMIZEBOX;
  windowStyle |= WS_THICKFRAME;

  // Obtain the size of the border.
  RECT border_rect = {0, 0, 0, 0};
  AdjustWindowRectEx(&border_rect, windowStyle, 0, windowExStyle);

  // In this case, the border rectangle is negative.
  windowX += border_rect.left;
  windowY += border_rect.top;

  // Grow by the size of the OS border.
  windowWidth += border_rect.right - border_rect.left;
  windowHeight += border_rect.bottom - border_rect.top;

  HWND handle = CreateWindowExA(
      windowExStyle, "tea_window_class", applicationName,
      windowStyle, windowX, windowY, windowWidth, windowHeight,
      0, 0, state->hInstance, 0);

  if (handle == 0) {
      MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

      TFATAL("Window creation failed!");
      return false;
  } else {
      state->handleWindow = handle;
  }

  // Show the window
  b32 shouldActivate = 1;  // TODO: if the window should not accept input, this should be false.
  i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
  // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
  ShowWindow(state->handleWindow, showWindowCommandFlags);

  // Clock setup
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  clockFrequency = 1.0 / (f64)frequency.QuadPart;
  QueryPerformanceCounter(&startTime);

  return true;
}

void platform_shutdown(void *platformState) {
  PlatformState *state = (PlatformState*)platformState;

  if(state->handleWindow) {
    DestroyWindow(state->handleWindow);
    state->handleWindow = 0;
  }
}

b8 platform_pump_message(void *platformState) {
  MSG message;
  while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
  return true;
}

void *platform_allocate(u64 size, b8 aligned) {
  return malloc(size);
}

void* platform_reallocate(void *block, u64 size, b8 aligned) {
  return realloc(block, size);
}

void platform_free(void *block, b8 aligned) {
  free(block);
}

void *platform_zero_memory(void *block, u64 size) {
  return memset(block, 0, size);
}

void *platform_copy_memory(void *dest, const void* source, u64 size) {
  return memcpy(dest, source, size);
}

void *platform_move_memory(void *dest, const void* source, u64 size) {
  return memmove(dest, source, size);
}

void *platform_set_memory(void *dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

void platform_console_write(const char *message, u8 color) {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  
  static u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(consoleHandle, levels[color]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD numberWritten = 0;
  WriteConsoleA(consoleHandle, message, (DWORD)length, numberWritten, 0);
}

void platform_console_write_error(const char *message, u8 color) {
  HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
  
  static u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(consoleHandle, levels[color]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD numberWritten = 0;
  WriteConsoleA(consoleHandle, message, (DWORD)length, numberWritten, 0);
}

f64 platform_get_absolute_time() {
  LARGE_INTEGER nowTime;
  QueryPerformanceCounter(&nowTime);
  return (f64)nowTime.QuadPart * clockFrequency;
}

void platform_sleep(u64 ms) {
  Sleep(ms);
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
  switch (msg) {
    case WM_ERASEBKGND:
        // Notify the OS that erasing will be handled by the application to prevent flicker.
        return 1;
    case WM_CLOSE:
        // TODO: Fire an event for the application to quit.
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: {
        // Get the updated size.
        // RECT r;
        // GetClientRect(hwnd, &r);
        // u32 width = r.right - r.left;
        // u32 height = r.bottom - r.top;

        // TODO: Fire an event for window resize.
    } break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      // Key pressed/released
      b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
      Keys key = (u16)w_param;

      input_process_key(key, pressed);

    } break;
    case WM_MOUSEMOVE: {
        // Mouse move
      i32 xPosition = GET_X_LPARAM(l_param);
      i32 yPosition = GET_Y_LPARAM(l_param);

      input_process_mouse_move(xPosition, yPosition);

    } break;
    case WM_MOUSEWHEEL: {
      i32 zDelta = GET_WHEEL_DELTA_WPARAM(w_param);
        if (zDelta != 0) {
          // Flatten the input to an OS-independent (-1, 1)
          zDelta = (zDelta < 0) ? -1 : 1;
          input_process_mouse_wheel(zDelta);
        }
    } break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP: {
      b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
      Buttons mouseButton = BUTTON_MAX_BUTTONS; 
      switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
          mouseButton = BUTTON_LEFT;
        break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
          mouseButton = BUTTON_RIGHT;
        break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP: 
          mouseButton = BUTTON_MIDDLE;
        break;
      }
      if(mouseButton != BUTTON_MAX_BUTTONS) {
        input_process_button(mouseButton, pressed);
      }
    } break;
  }
  return DefWindowProcA(hwnd, msg, w_param, l_param);
}
#endif
