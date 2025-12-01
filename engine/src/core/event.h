#pragma once

#include "../defines.h"

//128 bytes
typedef union {
  i64 int64[2];
  u64 uint64[2];
  f64 float64[2];

  i32 int32[4];
  u32 uint32[4];
  f32 float32[4];

  i16 int16[8];
  u16 uint16[8];

  i8 int8[16];
  u8 uint8[16];
} EventContext;


//returns true if handled the event
typedef b8 (*PFN_on_event)(u16 code, void* sender, void* listener, EventContext data);

b8 event_init();
void event_shutdown();

TAPI b8 event_register(u16 code, void *listener, PFN_on_event on_event);
TAPI b8 event_unregister(u16 code, void *listener, PFN_on_event on_event);
TAPI b8 event_fire(u16 code, void *sender, EventContext context);

typedef enum SystemEventCode {
  EVENT_CODE_APPLICATION_QUIT = 0x01,
  EVENT_CODE_KEY_PRESSED = 0x02,
  EVENT_CODE_KEY_RELEASED = 0x03,
  EVENT_CODE_BUTTON_PRESSED = 0x04,
  EVENT_CODE_BUTTON_RELEASED = 0x05,
  EVENT_CODE_MOUSE_MOVED = 0x06,
  EVENT_CODE_MOUSE_WHEEL = 0x06,
  EVENT_CODE_RESIZED = 0x07,
  MAX_EVENT_CODE = 0xFF
} SystemEventCode;
