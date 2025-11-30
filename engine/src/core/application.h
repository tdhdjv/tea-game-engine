#pragma once

#include "../defines.h"

struct Game;

typedef struct ApplicationConfig {
  i16 startPositionX;
  i16 startPositionY;

  i16 startWidth;
  i16 startHeight;

  char *name;
} ApplicationConfig;

TAPI b8 application_create(struct Game* gameInst);

TAPI b8 application_run();
