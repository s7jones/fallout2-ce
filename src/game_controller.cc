#include "game_controller.h"
#include <SDL_gamecontroller.h>
#include "debug.h"
#include <SDL.h>

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

    SDL_GameController* controller = findController();
    if (controller ==  nullptr) {
        debugPrint("failed!\n");
        return -1;
    }

    // ggameControllerInitialized = true;
    //_gcontroller_enabled = 1;

    // gameControllerSetCursor(MOUSE_CURSOR_ARROW);

    // SFALL
    // customControllerModeFrmsInit();

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

SDL_GameController* findController()
{
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return nullptr;
}

void _gcontroller_handle_event(int controllerX, int controllerY, int controllerState)
{
    //if (!gGameControllerInitialized) {
    //    return;
    //}

    //if (gGameControllerCursor >= MOUSE_CURSOR_WAIT_PLANET) {
    //    return;
    //}

    //if (!_gcontroller_enabled) {
    //    return;
    //}

    //if (_gcontroller_clicked_on_edge) {
    //    if (_gcontroller_get_click_to_scroll()) {
    //        return;
    //    }
    //}

    //if (gameControllerClickOnInterfaceBar()) {
    //    return;
    //}

    //if ((controllerState & MOUSE_EVENT_RIGHT_BUTTON_DOWN) != 0) {
    //    if ((controllerState & MOUSE_EVENT_RIGHT_BUTTON_REPEAT) == 0 && (gGameControllerHexCursor->flags & OBJECT_HIDDEN) == 0) {
    //        gameControllerCycleMode();
    //    }
    //    return;
    //}

    //if ((controllerState & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
    //    if (gGameControllerMode == GAME_MOUSE_MODE_MOVE) {
    //    }
    //}
}

} // namespace fallout
