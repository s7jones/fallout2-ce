#include "game_controller.h"

namespace fallout {

static int gControllerLastError;

int gameControllerInit()
{
    // if (ggameControllerInitialized) {
    //     return -1;
    // }

    // if (gameControllerObjectsInit() != 0) {
    //     return -1;
    // }

    // initialize direct sound
    if (sdlControllerInit(/*_detectDevices, 24, 0x8000, 0x8000, 22050*/) != 0) {
        // if (gGameSoundDebugEnabled) {
        debugPrint("failed!\n");
        //}

        return -1;
    }

    // ggameControllerInitialized = true;
    //_gcontroller_enabled = 1;

    // gameControllerSetCursor(MOUSE_CURSOR_ARROW);

    // SFALL
    // customMouseModeFrmsInit();

    return 0;
}

int sdlControllerInit()
{
    if (!controllerEngineInit()) {
        debugPrint("controllerInit: Unable to init controller engine\n");

        /*gSoundLastError = SOUND_SOS_DETECTION_FAILURE;*/
        gControllerLastError = CONTROLLER_SOS_DRIVER_NOT_LOADED;
        return gControllerLastError;
    }

    gControllerLastError = CONTROLLER_NO_ERROR;
    return gControllerLastError;
}

bool controllerEngineInit()
{
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == -1) {
        return false;
    }

    return true;
}

} // namespace fallout
