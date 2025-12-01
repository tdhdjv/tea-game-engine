#include "event.h"

#include "tmemory.h"
#include "../containers/darray.h"
#include "logger.h"

typedef struct RegisteredEvent {
  void *listener;
  PFN_on_event callback;
} RegisteredEvent;

typedef DARRAY(RegisteredEvent) EventCodeEntry;

#define MAX_MESSAGE_CODES 16304

typedef struct EventSystemState {
  EventCodeEntry registered[MAX_MESSAGE_CODES];
} EventSystemState;

static b8 isInitialized = false;
static EventSystemState state;

b8 event_init() {
  if(isInitialized) {
    return false;
  }
  tzero_memory(&state, sizeof(state));
  isInitialized = true;

  return true;
}

void event_shutdown() {
  for(u16 i = 0; i < MAX_EVENT_CODE; i++) {
    if(state.registered[i] != NULL) {
      darray_destroy(state.registered[i]);
      state.registered[i] = NULL;
    }
  }
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event) {
  if(!isInitialized) return false;

  if(state.registered[code] == NULL) {
    state.registered[code] = darray_create(RegisteredEvent, 1);
  }

  u64 registeredCount = darray_length(state.registered[code]);
  for(u64 i = 0; i < registeredCount; i++) {
    if(state.registered[code][i].listener == listener) {
      return false;
    }
  }
  RegisteredEvent event;
  event.listener = listener;
  event.callback = on_event;
  darray_append(state.registered[code], event);

  return true;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event) {
  if(!isInitialized) return false;

  if(state.registered[code] == NULL) {
    state.registered[code] = darray_create(RegisteredEvent, 1);
  }

  u64 registeredCount = darray_length(state.registered[code]);
  for(u64 i = 0; i < registeredCount; i++) {
    if(state.registered[code][i].listener == listener) {
      darray_remove(state.registered[code], i, NULL);
      return true;
    }
  }

  return false;
}

b8 event_fire(u16 code, void *sender, EventContext context) {
  if(!isInitialized) return false;

  if(state.registered[code] == NULL) {
    state.registered[code] = darray_create(RegisteredEvent, 1);
    
  }

  u64 registeredCount = darray_length(state.registered[code]);
  for(u64 i = 0; i < registeredCount; i++) {
    RegisteredEvent event = state.registered[code][i]; 
    
    if(event.callback(code, sender, event.listener, context)) {
      //event is handled then it is finished
      return true;
    }
  }

  return false;
}

