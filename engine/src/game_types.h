#pragma once

#include "core/application.h"

typedef struct Game {
  ApplicationConfig appConfig;

  b8 (*init)(struct Game *gameInstance);
  b8 (*update)(struct Game *gameInstance, f32 deltaTime);
  b8 (*render)(struct Game *gameInstance, f32 deltaTime);
  void (*on_resize)(struct Game *gameInstance, u32 width, u32 height);

  void* state;
} Game;
