#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

namespace fallout {

typedef enum ControllerError {
    CONTROLLER_NO_ERROR = 0,
    CONTROLLER_SOS_DRIVER_NOT_LOADED = 1,
};

int gameControllerInit();

} // namespace fallout

#endif /* GAME_CONTROLLER_H */
