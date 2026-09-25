# Controller Refactor TODO

This is the prioritized work list for the SDL2 controller refactor. The main menu is the first focused UI target.

## Completed / prototype work

- [x] Confirm that the project uses SDL2 and `SDL_GameController`.
- [x] Keep controller support optional when no controller is connected.
- [x] Handle controller add/remove events in the SDL event pump.
- [x] Avoid dereferencing a null controller handle.
- [x] Translate Western PlayStation Cross (SDL `A`) to `KEY_RETURN`.
- [x] Translate PlayStation Circle (SDL `B`) to `KEY_ESCAPE`.
- [x] Reserve Square (SDL `X`) and Triangle (SDL `Y`) for future secondary/special actions.
- [x] Translate the controller D-pad to existing arrow key codes.
- [x] Move the game cursor with the left stick.
- [x] Scroll the map with the right stick.
- [x] Add L1 attack mode and R1 movement mode shortcuts.
- [x] Process controller updates during the combat input loop.
- [x] Snap the attack cursor between enemies using left-stick direction.
- [x] Cycle combat targets with L2/R2.
- [x] Add a Square combat action menu for turn, combat, inventory, skills, and item actions.
- [x] Use L3/R3 for the controller-help overlay because Steam may steal Guide.
- [x] Add main menu focus state.
- [x] Navigate the main menu with arrow keys and D-pad input.
- [x] Activate the focused main menu item with Enter/Cross.
- [x] Show quit confirmation when Circle/Escape is pressed from the main menu.
- [x] Add a prototype focus visual using the pressed menu artwork.
- [x] Preserve existing mouse behavior and keyboard shortcuts.

## Immediate next steps

### Build and runtime verification

- [ ] Regenerate the build directory so it no longer references the missing Visual Studio CMake 3.25 installation.
- [ ] Build the modified sources on Windows.
- [ ] Verify the game starts without a controller.
- [ ] Verify the game starts with a controller already connected.
- [ ] Verify controller hot-plug and removal.
- [ ] Verify main menu arrow/D-pad navigation.
- [ ] Verify Cross activates Intro, New Game, Load Game, Options, Credits, and Exit as focus moves.
- [ ] Verify Circle/Escape opens the quit confirmation and does not immediately exit.
- [ ] Verify Cross confirms and Circle cancels the quit dialog.
- [ ] Verify L3/R3 opens and closes the controller-help overlay.
- [ ] Verify mouse selection and keyboard shortcuts remain unchanged.
- [ ] Test at least one PlayStation-style controller; Xbox-style SDL aliases may be tested separately.

### Stabilize the main menu prototype

- [ ] Decide which screens should use Square for secondary actions and Triangle for special actions.
- [ ] Add D-pad/axis repeat behavior without flooding the input queue.
- [ ] Reset focus deliberately when the main menu is opened or restored.
- [ ] Ensure mouse hover updates or clears keyboard/controller focus consistently.
- [ ] Make focused and pressed states visually distinct if the pressed artwork is confusing.
- [ ] Verify that the Shift+Credits/Quotes behavior is unaffected.
- [ ] Fix or review the existing main-menu button cleanup logic while touching this code.

## Layered character editor navigation

- [x] Define a panel-navigation layer for the character editor.
- [x] Add top-level focus targets for identity, stats, skills, optional traits, Options, and Done.
- [x] Add Cross-to-enter behavior for the focused panel.
- [x] Add Circle-to-return behavior from panel detail to panel navigation.
- [x] Adapt the existing `characterEditorSelectedItem` ranges behind the panel-detail layer.
- [x] Add dedicated focus borders for the active panel and active control.
- [x] Keep the editor Done action reachable from panel navigation.
- [ ] Verify character creation with both keyboard and controller input.

## Known issues to fix

### Combat enemy cycling

- [ ] Investigate combat target cycling when the current target is invalid, dead, off-screen, or no longer in the combat list.
- [ ] Make L2/R2 cycling consistently move the cursor to the selected enemy.
- [ ] Preserve a stable target index or target handle across combat turns.
- [ ] Reset the controller target when combat starts, ends, or the target dies.
- [ ] Ensure cycling skips allies, the player character, dead critters, and invalid targets.
- [ ] Add a visible target highlight or debug message when the selected enemy changes.
- [ ] Test cycling with enemies arranged horizontally, vertically, diagonally, and at different distances.

### Character editor navigation

- [ ] Simplify the character editor direction map so panel navigation follows the visible screen layout consistently.
- [ ] Avoid surprising diagonal jumps between Identity, Stats, Skills, Traits, Options, and Done.
- [ ] Define predictable behavior for pressing into an edge where no neighboring panel exists.
- [ ] Make the Stats/Skills/Trait panel relationships easier to understand from every direction.
- [ ] Add a small navigation diagram or controller-help text for the character editor.
- [ ] Verify that panel focus and control focus never become confused after returning with Circle.

## Generic focus system

- [ ] Define a logical action type for `NavigateUp`, `NavigateDown`, `NavigateLeft`, `NavigateRight`, `Confirm`, `Cancel`, and paging actions.
- [ ] Define a focusable-control representation independent of SDL.
- [ ] Define enabled/disabled state and skip disabled controls during navigation.
- [ ] Define focus gained/lost and activation callbacks.
- [ ] Decide between explicit neighbor links and geometry-based navigation.
- [ ] Support vertical lists, horizontal lists, grids, and wrapping behavior.
- [ ] Provide a focus indicator API rather than modifying button artwork directly.
- [ ] Add a modal-window focus context and a focus stack for nested dialogs.
- [ ] Keep mouse hover/click and keyboard shortcuts compatible with focused controls.
- [ ] Add a compatibility adapter for legacy screens that still consume integer key codes.

## Convert additional UI screens

Prioritize screens with clear list/button navigation:

- [ ] Main menu options screen.
- [ ] Generic dialog boxes.
- [ ] Yes/no and confirmation dialogs.
- [ ] Load/save game screens.
- [ ] Character selector.
- [ ] Character editor.
- [ ] Preferences/options controls.
- [ ] Pip-Boy tabs and lists.
- [ ] Inventory controls.
- [ ] Skilldex.
- [ ] Automap/world map controls.
- [ ] Combat interface controls.

Each converted screen should document its focus order, confirm action, cancel action, and any controls that still require cursor positioning.

## Input abstraction

- [ ] Introduce a controller manager API that owns handles and instance IDs.
- [ ] Move controller button translation out of individual screens.
- [ ] Add a logical action-to-key compatibility layer.
- [ ] Track button pressed/released state where held behavior matters.
- [ ] Add controller button/axis repeat timing.
- [ ] Track the last input method for UI prompt selection.
- [ ] Add optional loading of `gamecontrollerdb.txt`.
- [ ] Decide how mapping files are packaged and located on Windows, Linux, macOS, Android, and iOS.
- [ ] Add controller cleanup during game/window shutdown.
- [ ] Decide whether and when to support multiple controllers.

## Virtual cursor and gameplay

- [ ] Prototype left-stick cursor movement using `_mouse_simulate_input()`.
- [ ] Add a configurable analog dead zone.
- [ ] Rescale values outside the dead zone for smooth movement.
- [ ] Clamp virtual cursor movement to the game window.
- [ ] Map primary and secondary controller buttons to existing mouse behavior.
- [ ] Verify cursor movement, hover, click, drag, and repeat behavior.
- [ ] Handle controller and physical mouse input in the same frame without lost events.
- [ ] Define a clear distinction between UI focus navigation and virtual-cursor mode.
- [ ] Add right-stick or shoulder-button alternatives for scrolling where appropriate.
- [ ] Evaluate controller use in combat targeting and inventory.

## Quality-of-life features

- [ ] Add controller type detection with `SDL_GameControllerGetType()`.
- [ ] Add Xbox/PlayStation/Switch-aware button prompts.
- [ ] Add optional rumble through `SDL_GameControllerRumble()`.
- [ ] Add configurable action rebinding.
- [ ] Persist controller bindings using the existing settings system.
- [ ] Add a reset-to-default bindings option.
- [ ] Add a controller test/debug screen or diagnostic logging that can be enabled without per-frame spam.

## Testing and regression coverage

- [ ] Test no-controller startup.
- [ ] Test controller startup and hot-plug.
- [ ] Test disconnect/reconnect during a modal window.
- [ ] Test window focus loss/regain with held buttons.
- [ ] Test input queue overflow behavior during held/repeated input.
- [ ] Test disabled controls are skipped.
- [ ] Test every converted screen with keyboard, mouse, and controller.
- [ ] Test multiple controller models and mapping database behavior.
- [ ] Add automated unit tests for action mapping and focus movement where practical.
- [ ] Document manual controller test steps for contributors.
