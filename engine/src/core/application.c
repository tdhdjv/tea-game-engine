#include "application.h"
#include "event.h"
#include "logger.h"
#include "../platform/platform.h"
#include "../game_types.h"
#include "input.h"
#include "tmemory.h"

typedef struct ApplicationState {
  Game* gameInstance;
  b8 isRunning;
  b8 isSuspended;
  void *platformState;
  i16 width;
  i16 height;
  f64 previousTime;
} ApplicationState;

static b8 initialized = false;
static ApplicationState appState;

//Event handler
b8 application_on_event(u16 code, void *sender, void *listener, EventContext context);
b8 application_on_key(u16 code, void *sender, void *listener, EventContext context);

b8 application_create(Game* game) {
  TINFO(get_memory_usage_str());
  if(initialized) {
    TERROR("application_create called more than once");
    return false;
  }
  appState.gameInstance = game;
  //Initilize subsystem
  log_init();
  input_init();

  appState.isRunning = true;
  appState.isSuspended = false;

  if(!event_init()) {
    TERROR("Event system failed initialization. Application cannot continue");
    return false;
  }
  
  if(!platform_startup(
        &appState.platformState,
        game->appConfig.name,
        game->appConfig.startPositionX,
        game->appConfig.startPositionY,
        game->appConfig.startWidth,
        game->appConfig.startHeight)) {
    return false;
  }

  event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

  initialized = true;
  return true;
}

b8 application_run() {
  while(appState.isRunning) {
    if(!platform_pump_message(appState.platformState)) {
      appState.isRunning = false;
    }

    if(!appState.isSuspended) {
      if(!appState.gameInstance->update(appState.gameInstance, (f32)0)) {
        TFATAL("Game update failed. shutting down");
        appState.isRunning = false;
        break;
      }

      if(!appState.gameInstance->render(appState.gameInstance, (f32)0)) {
        TFATAL("Game render failed. shutting down");
        appState.isRunning = false;
        break;
      }

      input_update(0);
    }
  }

  appState.isRunning = false;

  event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

  event_shutdown();
  input_shutdown();

  platform_shutdown(appState.platformState);

  return true;
}

b8 application_on_event(u16 code, void *sender, void *listener, EventContext context) {
  switch(code) {
    case EVENT_CODE_APPLICATION_QUIT: {
      TINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down. \n");
      appState.isRunning = false;
      return true;
    }
  }
  return false;
}

b8 application_on_key(u16 code, void *sender, void *listener, EventContext context) {
  if(code == EVENT_CODE_KEY_PRESSED) {
    u16 keyCode = context.uint16[0];
    if(keyCode == KEY_ESCAPE) {
      EventContext data = {};
      event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
      return true;
    }
    else if(keyCode == KEY_A) {
      TDEBUG("Explicit: A key pressed!");
    }
    else {
      TDEBUG("'%c' key pressed in window", keyCode);
    }
  } else if(code == EVENT_CODE_KEY_RELEASED) {
    u16 keyCode = context.uint16[0];
    if(keyCode == KEY_B) {
      TDEBUG("Explicit: B key relased!");
    }
    else {
      TDEBUG("'%c' key released in window", keyCode);
    }
  }
  return false;
}