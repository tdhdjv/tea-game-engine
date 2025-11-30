#include "game.h"

#include <entry.h>

#include <platform/platform.h>

b8 create_game(Game* outGame) {
  outGame->appConfig.startPositionX = 100;
  outGame->appConfig.startPositionY = 100;
  outGame->appConfig.startWidth = 1200;
  outGame->appConfig.startHeight = 720;
  outGame->appConfig.name = "Tea Game Engine";
  outGame->init = game_init;
  outGame->update = game_update;
  outGame->render = game_render;
  outGame->on_resize = game_on_resize;

  outGame->state = tallocate(sizeof(GameState), MEMORY_TAG_GAME);

  return true;
}
