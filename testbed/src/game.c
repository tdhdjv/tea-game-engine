#include "game.h"
#include <core/logger.h>

b8 game_init(Game* gameInstance) {
  TDEBUG("Game Initalized!");
  return true;
}

b8 game_update(Game* gameInstance, f32 deltaTime) {
  return true;
}

b8 game_render(Game* gameInstance, f32 deltaTime) {
  return true;
}

void game_on_resize(Game* gameInstance, u32 width, u32 height) {
}
