#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <SDL_events.h>
#include <SDL_gamecontroller.h>

namespace fallout {

typedef enum ControllerError {
    CONTROLLER_NO_ERROR = 0,
    CONTROLLER_SOS_DRIVER_NOT_LOADED = 1,
};

int gameControllerInit();

int sdlControllerInit();

bool controllerEngineInit();

SDL_GameController* findController();

void onControllerAdded(SDL_Event event);

void onControllerRemoved(SDL_Event event);

void _gcontroller_handle_event();

} // namespace fallout

#endif /* GAME_CONTROLLER_H */
