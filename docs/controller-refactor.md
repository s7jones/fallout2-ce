# General Controller Refactor

## Purpose

Fallout 2 Community Edition is an SDL2-based reimplementation of Fallout 2. The original game is primarily mouse-driven, with keyboard shortcuts used throughout the UI. This refactor adds controller support without breaking the existing keyboard, mouse, touch, or platform-specific input paths.

The long-term goal is not merely to detect a gamepad. It is to make the game navigable and usable with a controller:

- Focusable windows and controls.
- Directional navigation with the D-pad, left stick, and keyboard arrows.
- A controller confirm action and a controller cancel action.
- Logical actions shared by keyboard, mouse, touch, and controller input.
- A virtual cursor for interfaces that fundamentally require mouse positioning.
- Optional support for multiple controllers, remapping, rumble, and controller-specific prompts.

The implementation should remain incremental. UI navigation is the first major milestone; full gameplay support comes afterward.

## Platform and API baseline

This project uses **SDL2**.

Use the SDL2 APIs:

- `SDL_GameController*`
- `SDL_INIT_GAMECONTROLLER`
- `SDL_CONTROLLERDEVICEADDED`
- `SDL_CONTROLLERDEVICEREMOVED`
- `SDL_CONTROLLERBUTTONDOWN`
- `SDL_CONTROLLERBUTTONUP`
- `SDL_CONTROLLERAXISMOTION`

Do not copy SDL3 examples directly. SDL3 uses renamed APIs such as `SDL_Gamepad*` and different event names.

SDL2's `SDL_GameController` abstraction is preferred over raw `SDL_Joystick` for ordinary controllers because it provides a standard layout for A/B/X/Y, D-pad, sticks, shoulders, triggers, and Start/Back. Raw joystick support can be added later for devices that do not fit the gamepad model.

## Existing input architecture

The relevant current flow is:

1. The platform startup code initializes SDL video, audio, and events.
2. The window manager initializes the keyboard and mouse input systems.
3. `inputGetInput()` calls `_GNW95_process_message()`.
4. `_GNW95_process_message()` drains the SDL event queue.
5. Keyboard events are converted through the keyboard logical-key tables.
6. Mouse events are converted into the existing mouse event state.
7. Input is placed in the game's input queue.
8. UI and gameplay code consume logical key codes through functions such as `gameHandleKey()`.

Important files:

- `src/input.cc` — SDL event pump and input queue integration.
- `src/input.h` — input queue and ticker declarations.
- `src/kb.cc` / `src/kb.h` — keyboard state and logical key conversion.
- `src/mouse.cc` / `src/mouse.h` — mouse state and virtual mouse simulation.
- `src/dinput.cc` / `src/dinput.h` — platform-independent input device layer.
- `src/game_controller.cc` / `src/game_controller.h` — current controller support.
- `src/mainmenu.cc` — first controller-focused UI implementation.
- `src/window_manager.cc` / `src/window_manager.h` — windows and mouse-oriented buttons.
- `src/game.cc` — game initialization and existing game-level input handling.

The existing keyboard path is valuable because many old UI screens already understand logical key codes such as:

- `KEY_ARROW_UP`
- `KEY_ARROW_DOWN`
- `KEY_ARROW_LEFT`
- `KEY_ARROW_RIGHT`
- `KEY_RETURN`
- `KEY_ESCAPE`

Controller UI navigation should reuse these codes temporarily where that gives correct behavior. A more explicit action layer should be introduced as the refactor expands.

## Current controller implementation

The repository already contained a partial controller implementation in `src/game_controller.cc`.

It currently:

- Initializes the SDL2 game-controller subsystem.
- Finds the first connected SDL game controller.
- Opens a controller when `SDL_CONTROLLERDEVICEADDED` is received.
- Replaces a removed controller with another available controller when possible.
- Routes button events from the SDL event pump into the existing input queue.
- Does not fail game initialization when no controller is connected.

The current first-pass mapping uses PlayStation terminology for user-facing behavior. SDL's standardized button names are included in parentheses:

| PlayStation input | SDL2 button | Existing/logical input |
|---|---|---|
| Cross | `SDL_CONTROLLER_BUTTON_A` | `KEY_RETURN` / confirm |
| Circle | `SDL_CONTROLLER_BUTTON_B` | `KEY_ESCAPE` / cancel/back |
| Square | `SDL_CONTROLLER_BUTTON_X` | Secondary action; unassigned for now |
| Triangle | `SDL_CONTROLLER_BUTTON_Y` | Special action; unassigned for now |
| D-pad up | `SDL_CONTROLLER_BUTTON_DPAD_UP` | `KEY_ARROW_UP` |
| D-pad down | `SDL_CONTROLLER_BUTTON_DPAD_DOWN` | `KEY_ARROW_DOWN` |
| D-pad left | `SDL_CONTROLLER_BUTTON_DPAD_LEFT` | `KEY_ARROW_LEFT` |
| D-pad right | `SDL_CONTROLLER_BUTTON_DPAD_RIGHT` | `KEY_ARROW_RIGHT` |
| L1 | `SDL_CONTROLLER_BUTTON_LEFTSHOULDER` | Enter attack/crosshair mode |
| R1 | `SDL_CONTROLLER_BUTTON_RIGHTSHOULDER` | Return to movement mode |
| L2 | `SDL_CONTROLLER_AXIS_TRIGGERLEFT` | Swap active hands outside combat; previous enemy target in combat |
| R2 | `SDL_CONTROLLER_AXIS_TRIGGERRIGHT` | Cycle active item action outside combat; next enemy target in combat |
| L3 or R3 | `SDL_CONTROLLER_BUTTON_LEFTSTICK` / `RIGHTSTICK` | Toggle controller-help overlay |

The mapping follows Western PlayStation conventions: Cross confirms, Circle cancels, Square is reserved for secondary actions, and Triangle is reserved for special actions. It is intentionally small and is enough to exercise focused UI navigation while retaining the game's existing keyboard and mouse behavior. The physical Guide/PlayStation button is not used because Steam may intercept it.

The old `_gcontroller_handle_event()` implementation printed button state every frame and could dereference a null controller. Button events are now handled from the SDL event pump instead. The polling function is retained as a future location for analog input and controller state updates.

## SDL event lifecycle

The SDL event pump is in `_GNW95_process_message()` in `src/input.cc`. Controller lifecycle and button events must be handled there, alongside keyboard, mouse, touch, window, and quit events.

The important lifecycle rules are:

- A controller is optional.
- Do not treat “no controller connected” as a fatal initialization error.
- The device index in `SDL_CONTROLLERDEVICEADDED` is not a stable controller identity.
- `SDL_CONTROLLERDEVICEREMOVED` supplies the joystick instance ID.
- Store or compare the controller's `SDL_JoystickID` when handling removal.
- Close the SDL controller handle before replacing it.
- Do not call `SDL_GameControllerGetButton()` on a null handle.
- Avoid opening the same device more than once.

The current code supports one active controller. That is sufficient for initial UI work. A future multi-controller design should maintain a collection keyed by instance ID and assign controllers to player slots.

## Mapping database

SDL2 has built-in controller mappings, but uncommon devices may have incorrect or missing mappings. A current `gamecontrollerdb.txt` from the SDL community database can optionally be loaded with:

```cpp
SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
```

Loading the file should be optional. A missing mapping database must not prevent the game from starting. The runtime location and packaging of this file still need to be decided for each supported platform.

Potential locations include:

- The game executable/data directory.
- A platform-specific data directory.
- A user configuration directory.

The mapping database should not be hard-coded to a developer checkout path.

## UI navigation model

The central UI concept is a **focused window** containing zero or more **focusable controls**.

A focusable control should eventually provide:

- A visible focused/unfocused state.
- An enabled/disabled state.
- A logical activation action.
- A logical cancel action where appropriate.
- A navigation relationship to neighboring controls, or enough geometry to derive one.
- Optional wrapping behavior.
- Optional callbacks for focus gained/lost.

The navigation layer should not know whether input came from a keyboard, controller, or another device. It should receive logical navigation actions such as:

```text
NavigateUp
NavigateDown
NavigateLeft
NavigateRight
Confirm
Cancel
PageUp
PageDown
Back
```

For the first implementation, keyboard arrow key codes and controller button events can be translated into existing logical key codes. As more windows are converted, these actions should become an explicit input abstraction rather than relying on character/key constants.

### Focus behavior

Recommended default behavior:

- Focus starts on the first enabled control.
- Up/down navigation moves through a vertical list.
- Left/right navigation moves through a horizontal list or wraps in a vertical list when that is useful.
- Disabled controls are skipped.
- Navigation can wrap from the first control to the last and vice versa.
- Confirm activates only the focused control.
- Cancel closes the current modal window or returns to the previous screen.
- Keyboard shortcuts remain available for compatibility.
- Mouse hover and clicking remain available and may update the focused control.

Focus should be visibly different from activation. A focused button must not execute its callback merely because it became focused.

## Main menu implementation

The main menu is the first target because it is a small, clearly defined vertical list.

The menu contains:

1. Intro
2. New Game
3. Load Game
4. Options
5. Credits
6. Exit

The original implementation creates graphic buttons with `buttonCreate()`, draws the labels directly into the window buffer, and recognizes keyboard shortcuts such as `N`, `L`, `O`, and `E`.

The first controller-accessible implementation adds:

- A `gMainMenuFocusedButton` index.
- Focus movement using arrow keys and controller D-pad input.
- Wrapping focus movement.
- `KEY_RETURN` activation of the focused item.
- Western PlayStation Cross translated from SDL `A` to `KEY_RETURN`.
- PlayStation Circle translated from SDL `B` to `KEY_ESCAPE`.
- PlayStation Square and Triangle reserved for future secondary and special actions.
- A visual focus state using the menu's pressed button artwork.
- A quit confirmation dialog when Circle/Escape is pressed from the main menu.
- A debug controller-help overlay toggled by L3/R3.
- Preservation of existing mouse behavior and keyboard shortcuts.

This is intentionally local to `mainmenu.cc` at first. The code should be generalized only after the focus behavior is proven on the main menu.

### Main menu concerns

The main menu currently mixes several input mechanisms:

- Keyboard shortcut selection.
- Mouse button selection.
- Button hover and click processing in the window manager.
- Screensaver timeout handling.
- Special credits/quotes behavior while Shift is held.

Controller navigation must not remove any of these behaviors. In particular, Enter/Cross activation should select the focused item, while keyboard shortcuts should continue to select their explicitly named item even if a different item has focus. Circle/Escape from the main menu opens the standard “Are you sure you want to quit?” confirmation dialog instead of immediately leaving.

The current focus visual uses the pressed artwork as a focus indicator. This is a prototype. A better visual may eventually be needed so that “focused” and “pressed” are clearly distinct.

### Controller-help overlay

Pressing L3 or R3 opens a small floating debug window in the top-right corner listing the current PlayStation-centric controls. The window can be dragged by its background, and pressing either stick again closes it. The overlay is currently non-modal and exists to make controller work easier to test while the input system is being refactored. The Steam-intercepted Guide/PlayStation button is deliberately not used.

## Logical action abstraction

Directly calling `SDL_GameControllerGetButton()` from individual screens should be avoided. The desired layering is:

```text
SDL keyboard events ─┐
SDL controller events ├─> input/action translation ─> UI/game actions
SDL mouse events    ─┤
SDL touch events    ─┘
```

A possible logical action enum is:

```cpp
enum class InputAction {
    None,
    NavigateUp,
    NavigateDown,
    NavigateLeft,
    NavigateRight,
    Confirm,
    Cancel,
    Back,
    PageUp,
    PageDown,
    PrimaryClick,
    SecondaryClick,
};
```

The existing game still expects integer key codes in many places, so introducing this enum should be incremental. A practical transition is:

1. Continue translating controller buttons to existing key codes for converted screens.
2. Introduce actions for the generic focus manager.
3. Convert screens from key-code switches to action handling as they are updated.
4. Keep compatibility adapters for old screens.

## Layered character editor navigation

The character creation/editor screen is a good candidate for a two-level focus model because it is composed of several visual panels. The existing implementation already has a large `characterEditorSelectedItem` range and keyboard navigation, but controller confirm currently follows the legacy “finish editor” path instead of entering a panel.

The first layered controller implementation is now in place for character creation:

```text
Panel navigation level
  Identity: name, age, and gender
  Stats panel
  Skills panel
  Optional traits panel
  Options and Done actions

Panel detail level
  Cross enters the focused panel.
  D-pad/left stick navigates controls within the panel.
  Cross activates or edits the focused control.
  Circle returns to panel navigation.
  Circle from panel navigation returns to the previous screen.
```

The panel navigation uses the existing editor selection indices behind a small compatibility layer. Identity focus exposes name, age, and gender with the original pressed button artwork. Stats use the existing stat navigation and adjustment behavior. Skills and optional traits use their existing lists, with Cross toggling the selected skill/trait. Options and Done continue through their existing paths and validation.

This is the first use of a focus layer and should be generalized before other complex screens are converted. The creation editor now draws a focus border around the active panel and uses the pressed artwork for identity, Options, and Done controls. Inside Stats, Skills, and Traits, the existing selected-item highlighting identifies the active control.

The character selector demonstrates the intended visual language: the character biography/face/stats area and its previous/next arrows are one focus region, while the action buttons below are separate focus targets. Focus borders are restored before drawing the next border so the old target is not left highlighted.

## Mouse-centric gameplay

UI navigation alone does not solve Fallout's gameplay input. The game uses a precise cursor for:

- The isometric map.
- Interface bar actions.
- Inventory.
- Dialog controls.
- Object selection.
- Combat targeting.
- World map interaction.

The existing mouse layer exposes `_mouse_simulate_input()`, which can move the current cursor and simulate left/right mouse button state. This provides a possible foundation for a controller virtual cursor.

The current prototype uses the left stick to move the existing Fallout cursor and the right stick to scroll the map. Cross produces a virtual primary-click release at the cursor, so movement mode uses it to send the player there and attack mode uses it to attack the selected target. L1 enters attack/crosshair mode and R1 returns to movement mode. In combat, the left stick snaps between living enemy critters based on their relative screen direction while in attack mode; L2/R2 cycle the previous/next enemy target. Outside combat, L2 swaps active hands and R2 cycles the active item action as temporary interface shortcuts.

Pressing Square during combat opens a controller action menu with:

- End turn.
- End combat.
- Open inventory.
- Open skills.
- Cycle the current item action.
- Back.

D-pad/arrows navigate the menu, Cross confirms, and Circle or Square closes it.

A future virtual cursor should:

- Use the left stick or another configurable stick.
- Apply a dead zone.
- Scale movement based on stick magnitude.
- Clamp to the game window.
- Preserve the existing cursor and mouse event pipeline.
- Map a controller button to primary click and another to secondary click.
- Support repeat/hold behavior using the existing mouse repeat logic where possible.
- Avoid fighting with real mouse movement in the same frame.

Analog triggers should be read as axes, not treated only as digital buttons. Rumble can be added later through `SDL_GameControllerRumble()`.

## Dead zones and analog input

A typical SDL2 axis ranges from approximately `-32768` to `32767`. A dead zone is needed because a resting stick rarely reports exactly zero.

A basic normalization strategy:

```cpp
constexpr int kDeadZone = 8000;

if (value > -kDeadZone && value < kDeadZone) {
    value = 0;
}
```

A better implementation rescales the range outside the dead zone so movement begins smoothly instead of jumping at the threshold. Analog input should be polled once per frame after SDL events are processed.

Controller button events are appropriate for discrete actions. Polled button/axis state is appropriate for continuous movement, cursor movement, and held actions.

## Multiple controllers

The current implementation intentionally uses the first available controller. This is adequate for single-player UI navigation.

If local co-op or player assignment becomes necessary, the controller layer should maintain:

- Controller handle.
- Stable SDL joystick instance ID.
- Player slot.
- Connection state.
- Last-used input timestamp.
- Optional rumble state.

Do not use the connection device index as a persistent player identity because it can change when devices are added or removed.

## Input method and UI prompts

A future UI can track the last input method:

```text
Keyboard/mouse -> keyboard or mouse prompts
Controller      -> controller prompts
Touch           -> touch prompts
```

SDL2 can provide controller type information through `SDL_GameControllerGetType()`. This can eventually support Xbox, PlayStation, and Nintendo-style button glyphs. It should not block the initial focus/navigation implementation.

## Configuration and rebinding

Hard-coded mappings are acceptable for the first prototype, but the long-term system should support:

- Default controller mappings.
- User-rebindable actions.
- Per-device mappings when necessary.
- Separate UI and gameplay bindings.
- Reset-to-default behavior.
- Persisted settings through the existing settings/configuration system.

The abstraction should store actions rather than SDL button constants in gameplay code.

## Error handling and cleanup

Controller support must be optional and non-fatal:

- SDL controller subsystem initialization failure should be logged but should not prevent keyboard/mouse gameplay unless the whole SDL initialization failed.
- No controller connected is normal.
- A controller can be connected after the game starts.
- A disconnected controller must be closed and removed from active state.
- Shutdown must close any active controller handles.
- Mapping database load failure should be non-fatal.

## Testing strategy

Every controller change should be tested with no controller connected first, because that is the normal case for many users.

Minimum test matrix:

1. Game starts with no controller.
2. Game starts with a controller already connected.
3. Controller connects while the game is running.
4. Controller disconnects while the game is running.
5. Main menu focus moves up and down.
6. Focus wraps correctly.
7. Cross activates the focused main menu item.
8. Circle opens the main-menu quit confirmation.
9. L3/R3 toggles the controller-help overlay.
10. Existing keyboard shortcuts still work.
11. Existing mouse hover and click behavior still works.
12. Holding a D-pad direction does not flood or corrupt the input queue.
13. A controller with no community mapping still behaves safely.
14. Focus skips disabled controls once disabled controls are supported.
15. Window focus loss and regain do not leave stuck controller or mouse state.

The current local build directory cannot currently be relied upon because it references a missing Visual Studio CMake 3.25 installation. Build verification should be repeated after fixing or regenerating the build directory.

## Non-goals for the first milestone

The first milestone does not need to include:

- Full gameplay virtual cursor support.
- Multiple simultaneous controllers.
- Controller-specific button glyphs.
- Rumble.
- Complete rebinding UI.
- Raw joystick support.
- SDL3 support.

Those features should follow after the generic focus model and main menu behavior are stable.
