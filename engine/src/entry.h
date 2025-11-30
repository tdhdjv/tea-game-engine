#include "core/application.h"
#include "core/logger.h"
#include "core/tmemory.h"
#include "game_types.h"

extern b8 create_game(Game* outGame);

int main(void) {

  memory_init();

  Game gameInstance;

  if(!create_game(&gameInstance)) {
    TFATAL("Could not create Game!");
    return -1;
  }

  if(!gameInstance.init || !gameInstance.update || !gameInstance.render || !gameInstance.on_resize) {
    TFATAL("Game Function Pointers should be assigned!");
    return -1;
  }


  if(!application_create(&gameInstance)) {
    TFATAL("Application failed to create!");
    return -1;
  }

  if(!application_run()) {
    TFATAL("Application did not shutdown correctly");
    return -1;
  }

  memory_shutdown();

  return 0;
}
