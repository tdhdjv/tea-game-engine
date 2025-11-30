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

  event_shutdown();
  input_shutdown();

  platform_shutdown(appState.platformState);

  return true;
}
