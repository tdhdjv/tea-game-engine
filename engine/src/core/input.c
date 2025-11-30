#include "input.h"
#include "event.h"
#include "tmemory.h"
#include "logger.h"

typedef struct KeyboardState {
  b8 keys[256];
} KeyboardState;

typedef struct MouseState {
  i16 x;
  i16 y;
  u8 buttons[BUTTON_MAX_BUTTONS];
} MouseState;

typedef struct InputState {
  KeyboardState keyboardCurrent;
  KeyboardState keyboardPrevious;
  MouseState mouseCurrent;
  MouseState mousePrevious;
} InputState;

static b8 initalized = false;
static InputState state;

void input_init() {
  tzero_memory(&state, sizeof(InputState));
  initalized = true;
  TINFO("Input subsystem initialized");
}

void input_shutdown() {
  initalized = false;
}

void input_update(f64 deltaTime) {
  if(!initalized) {
    return;
  }

  tcopy_memory(&state.keyboardPrevious, &state.keyboardCurrent, sizeof(KeyboardState));
  tcopy_memory(&state.mousePrevious, &state.mouseCurrent, sizeof(MouseState));
}

void input_process_key(Keys key, b8 pressed) {
  if(state.keyboardCurrent.keys[key] != pressed) {
    state.keyboardCurrent.keys[key] = pressed;

    //Fire off an event
    EventContext context;
    context.uint16[0] = key;
    event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
  }
}

void input_process_button(Buttons button, b8 pressed) {
  if(state.mouseCurrent.buttons[button] != pressed) {
    state.mouseCurrent.buttons[button] = pressed;

    //Fire off an event
    EventContext context;
    context.uint16[0] = button;
    event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, NULL, context);
  }
}

void input_process_mouse_move(i16 x, i16 y) {
  if(state.mouseCurrent.x != x || state.mouseCurrent.y != y) {
    state.mouseCurrent.x = x;
    state.mouseCurrent.y = y;

    EventContext context;
    context.uint16[0] = x;
    context.uint16[1] = y;
    event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
  }
}

void input_process_mouse_wheel(i8 zDelta) {
  EventContext context;
  context.int8[0] = zDelta;
  event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_is_key_down(Keys key) {
  if(!initalized) return false;
  return state.keyboardCurrent.keys[key];
}

b8 input_is_key_up(Keys key) {
  if(!initalized) return true;
  return !state.keyboardCurrent.keys[key];
}

b8 input_was_key_down(Keys key) {
  if(!initalized) return false;
  return state.keyboardPrevious.keys[key];
}

b8 input_was_key_up(Keys key) {
  if(!initalized) return true;
  return !state.keyboardPrevious.keys[key];
}

b8 input_is_button_down(Buttons button) {
  if(!initalized) return false;
  return state.mouseCurrent.buttons[button];
}

b8 input_is_button_up(Buttons button) {
  if(!initalized) return true;
  return !state.mouseCurrent.buttons[button];
}

b8 input_was_button_down(Buttons button) {
  if(!initalized) return false;
  return state.mousePrevious.buttons[button];
}

b8 input_was_button_up(Buttons button) {
  if(!initalized) return true;
  return !state.mousePrevious.buttons[button];
}

void input_get_mouse_position(i32 *x, i32 *y) {
  if(!initalized) {
    *x = 0;
    *y = 0;
    return;
  }
  *x = state.mouseCurrent.x;
  *y = state.mouseCurrent.y;
}

void input_get_previous_mouse_position(i32 *x, i32 *y) {
  if(!initalized) {
    *x = 0;
    *y = 0;
    return;
  }
  *x = state.mousePrevious.x;
  *y = state.mousePrevious.y;
}
