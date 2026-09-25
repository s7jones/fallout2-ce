#include "game_controller.h"

#include <SDL.h>
#include <SDL_gamecontroller.h>

#include "color.h"
#include "combat.h"
#include "debug.h"
#include "game_mouse.h"
#include "input.h"
#include "interface.h"
#include "kb.h"
#include "map.h"
#include "mouse.h"
#include "svga.h"
#include "text_font.h"
#include "window_manager.h"

namespace fallout {

static int gControllerLastError;

static SDL_GameController* gcontroller;

static int gControllerDebugWindow = -1;
static bool gControllerDebugWindowVisible = false;
static bool gControllerCrossDown = false;
static bool gControllerLeftShoulderDown = false;
static bool gControllerRightShoulderDown = false;
static bool gControllerLeftTriggerDown = false;
static bool gControllerRightTriggerDown = false;
static bool gControllerCombatMenuOpen = false;
static int gControllerTargetDirectionX = 0;
static int gControllerTargetDirectionY = 0;
static unsigned int gControllerTargetNextMove = 0;

static void controllerDebugWindowCreate()
{
    if (gControllerDebugWindow != -1) {
        return;
    }

    constexpr int width = 320;
    constexpr int height = 250;
    constexpr int margin = 8;
    int x = screenGetWidth() - width - margin;
    int y = margin;
    if (x < margin) {
        x = margin;
    }

    gControllerDebugWindow = windowCreate(x,
        y,
        width,
        height,
        _colorTable[0],
        WINDOW_MOVE_ON_TOP | WINDOW_DRAGGABLE_BY_BACKGROUND);
    if (gControllerDebugWindow == -1) {
        return;
    }

    windowDrawBorder(gControllerDebugWindow);

    int textColor = _colorTable[21091] | 0x06000000;
    int lineHeight = fontGetLineHeight();
    windowDrawText(gControllerDebugWindow, "CONTROLLER CONTROLS", 0, 12, 10, textColor);
    windowDrawText(gControllerDebugWindow, "D-pad / Arrows: Navigate", 0, 12, 10 + lineHeight * 2, textColor);
    windowDrawText(gControllerDebugWindow, "Cross (A): Select / Continue", 0, 12, 10 + lineHeight * 3, textColor);
    windowDrawText(gControllerDebugWindow, "Circle (B): Back / Cancel", 0, 12, 10 + lineHeight * 4, textColor);
    windowDrawText(gControllerDebugWindow, "Square (X): Combat menu / secondary", 0, 12, 10 + lineHeight * 5, textColor);
    windowDrawText(gControllerDebugWindow, "Triangle (Y): Special action", 0, 12, 10 + lineHeight * 6, textColor);
    windowDrawText(gControllerDebugWindow, "Left stick: Move cursor", 0, 12, 10 + lineHeight * 7, textColor);
    windowDrawText(gControllerDebugWindow, "Right stick: Scroll map", 0, 12, 10 + lineHeight * 8, textColor);
    windowDrawText(gControllerDebugWindow, "L1/R1: Attack / Move mode", 0, 12, 10 + lineHeight * 9, textColor);
    windowDrawText(gControllerDebugWindow, "L2: Swap hands  R2: Cycle action", 0, 12, 10 + lineHeight * 10, textColor);
    windowDrawText(gControllerDebugWindow, "L3 / R3: Toggle this help", 0, 12, 10 + lineHeight * 11, textColor);

    windowRefresh(gControllerDebugWindow);
    gControllerDebugWindowVisible = true;
}

static void controllerDebugWindowToggle()
{
    if (gControllerDebugWindow == -1) {
        controllerDebugWindowCreate();
        return;
    }

    if (gControllerDebugWindowVisible) {
        windowHide(gControllerDebugWindow);
    } else {
        windowShow(gControllerDebugWindow);
        windowRefresh(gControllerDebugWindow);
    }

    gControllerDebugWindowVisible = !gControllerDebugWindowVisible;
}

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

    // A controller is optional. It may also be connected later via
    // SDL_CONTROLLERDEVICEADDED.
    gcontroller = findController();

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

void onControllerAdded(SDL_Event event)
{
    if (!gcontroller) {
        gcontroller = SDL_GameControllerOpen(event.cdevice.which);
    }
}

void onControllerRemoved(SDL_Event event)
{
    if (gcontroller && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gcontroller))) {
        SDL_GameControllerClose(gcontroller);
        gcontroller = findController();
        gControllerCrossDown = false;
        gControllerLeftShoulderDown = false;
        gControllerRightShoulderDown = false;
        gControllerLeftTriggerDown = false;
        gControllerRightTriggerDown = false;
        gControllerTargetDirectionX = 0;
        gControllerTargetDirectionY = 0;
        gControllerTargetNextMove = 0;
    }
}

void onControllerButtonDown(SDL_Event event)
{
    if (gcontroller == nullptr || event.cbutton.state != SDL_PRESSED) {
        return;
    }

    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(gcontroller);
    if (joystick == nullptr || event.cbutton.which != SDL_JoystickInstanceID(joystick)) {
        return;
    }

    switch (static_cast<SDL_GameControllerButton>(event.cbutton.button)) {
    case SDL_CONTROLLER_BUTTON_A:
        // Outside combat, Cross is the normal UI confirm action. During
        // combat it is handled as a mouse click by the controller update so
        // it can move or attack instead of ending the player's turn. While
        // the combat menu is open it is a menu confirmation instead.
        enqueueInputEvent(gControllerCombatMenuOpen || isInCombat()
                ? CONTROLLER_INPUT_COMBAT_CONFIRM
                : KEY_RETURN);
        break;
    case SDL_CONTROLLER_BUTTON_X:
        // Square opens the combat action menu, or closes it when it is already
        // open.
        if (gControllerCombatMenuOpen || isInCombat()) {
            enqueueInputEvent(CONTROLLER_INPUT_COMBAT_MENU);
        }
        break;
    case SDL_CONTROLLER_BUTTON_B:
        // Controller cancel/back.
        enqueueInputEvent(KEY_ESCAPE);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        enqueueInputEvent(KEY_ARROW_UP);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        enqueueInputEvent(KEY_ARROW_DOWN);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        enqueueInputEvent(KEY_ARROW_LEFT);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        enqueueInputEvent(KEY_ARROW_RIGHT);
        break;
    default:
        break;
    }
}

void gameControllerSetCombatMenuOpen(bool open)
{
    gControllerCombatMenuOpen = open;
}

void onControllerHelpButtonPressed(SDL_Event event)
{
    if (gcontroller == nullptr || event.cbutton.state != SDL_PRESSED) {
        return;
    }

    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(gcontroller);
    if (joystick == nullptr || event.cbutton.which != SDL_JoystickInstanceID(joystick)) {
        return;
    }

    SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(event.cbutton.button);
    if (button == SDL_CONTROLLER_BUTTON_LEFTSTICK
        || button == SDL_CONTROLLER_BUTTON_RIGHTSTICK) {
        controllerDebugWindowToggle();
    }
}

static int controllerAxisToMouse(Sint16 value)
{
    constexpr int deadZone = 8000;
    constexpr int maxSpeed = 8;

    int magnitude = value < 0 ? -value : value;
    if (magnitude <= deadZone) {
        return 0;
    }

    int speed = 1 + (magnitude - deadZone) * (maxSpeed - 1) / (32767 - deadZone);
    return value < 0 ? -speed : speed;
}

static int controllerAxisToDirection(Sint16 value)
{
    constexpr int deadZone = 10000;
    if (value > deadZone) {
        return 1;
    }
    if (value < -deadZone) {
        return -1;
    }
    return 0;
}

void _gcontroller_handle_event()
{
    if (gcontroller == nullptr) {
        return;
    }

    // The left stick controls the existing Fallout cursor. This deliberately
    // goes through the mouse layer so the normal hover and cursor logic keeps
    // working.
    Sint16 leftX = SDL_GameControllerGetAxis(gcontroller, SDL_CONTROLLER_AXIS_LEFTX);
    Sint16 leftY = SDL_GameControllerGetAxis(gcontroller, SDL_CONTROLLER_AXIS_LEFTY);
    int cursorDx = controllerAxisToMouse(leftX);
    int cursorDy = controllerAxisToMouse(leftY);

    // The right stick scrolls the map independently of the cursor.
    Sint16 rightX = SDL_GameControllerGetAxis(gcontroller, SDL_CONTROLLER_AXIS_RIGHTX);
    Sint16 rightY = SDL_GameControllerGetAxis(gcontroller, SDL_CONTROLLER_AXIS_RIGHTY);
    int scrollX = controllerAxisToDirection(rightX);
    int scrollY = controllerAxisToDirection(rightY);
    if (scrollX != 0 || scrollY != 0) {
        mapScroll(scrollX, scrollY);
    }

    bool crossDown = SDL_GameControllerGetButton(gcontroller, SDL_CONTROLLER_BUTTON_A) != 0;
    if (cursorDx != 0 || cursorDy != 0) {
        if (isInCombat() && gameMouseGetMode() == GAME_MOUSE_MODE_CROSSHAIR) {
            int targetDirectionX = controllerAxisToDirection(leftX);
            int targetDirectionY = controllerAxisToDirection(leftY);
            unsigned int now = getTicks();
            if (targetDirectionX != gControllerTargetDirectionX
                || targetDirectionY != gControllerTargetDirectionY
                || now >= gControllerTargetNextMove) {
                combatControllerMoveTarget(targetDirectionX, targetDirectionY);
                gControllerTargetNextMove = now + 220;
            }
            gControllerTargetDirectionX = targetDirectionX;
            gControllerTargetDirectionY = targetDirectionY;
        } else {
            _mouse_simulate_input(cursorDx, cursorDy, 0);
            gControllerTargetDirectionX = 0;
            gControllerTargetDirectionY = 0;
        }
    }

    if (crossDown != gControllerCrossDown) {
        int mouseX;
        int mouseY;
        mouseGetPosition(&mouseX, &mouseY);
        _gmouse_handle_event(mouseX,
            mouseY,
            crossDown ? MOUSE_EVENT_LEFT_BUTTON_DOWN : MOUSE_EVENT_LEFT_BUTTON_UP);
    }

    // L1 enters attack/target mode and R1 returns to movement mode. The
    // existing game mouse implementation then performs the actual attack
    // when the virtual cursor is clicked.
    bool leftShoulderDown = SDL_GameControllerGetButton(gcontroller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;
    bool rightShoulderDown = SDL_GameControllerGetButton(gcontroller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;
    if (leftShoulderDown && !gControllerLeftShoulderDown) {
        gameMouseSetMode(GAME_MOUSE_MODE_CROSSHAIR);
    }
    if (rightShoulderDown && !gControllerRightShoulderDown) {
        gameMouseSetMode(GAME_MOUSE_MODE_MOVE);
    }

    // Triggers are temporary shortcuts for hand/action selection until the
    // interface bar gets its own controller focus layer.
    bool leftTriggerDown = SDL_GameControllerGetAxis(gcontroller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000;
    bool rightTriggerDown = SDL_GameControllerGetAxis(gcontroller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000;
    if (leftTriggerDown && !gControllerLeftTriggerDown) {
        if (isInCombat()) {
            combatControllerCycleTarget(-1);
        } else if (interfaceBarEnabled()) {
            interfaceBarSwapHands(true);
        }
    }
    if (rightTriggerDown && !gControllerRightTriggerDown) {
        if (isInCombat()) {
            combatControllerCycleTarget(1);
        } else if (interfaceBarEnabled()) {
            interfaceCycleItemAction();
        }
    }

    gControllerCrossDown = crossDown;
    gControllerLeftShoulderDown = leftShoulderDown;
    gControllerRightShoulderDown = rightShoulderDown;
    gControllerLeftTriggerDown = leftTriggerDown;
    gControllerRightTriggerDown = rightTriggerDown;
}

} // namespace fallout
