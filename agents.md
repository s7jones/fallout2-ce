# Agent Notes

This repository is an ongoing refactor of **Fallout 2 Community Edition** to add usable SDL2 controller support while preserving the existing keyboard and mouse behavior.

The controller work is intentionally incremental. The first target is controller-accessible UI, beginning with the main menu, before extending navigation to dialogs, options, inventory, save/load screens, and gameplay's mouse-centric interface.

## Important references

- [General controller refactor](docs/controller-refactor.md) — comprehensive architecture notes, current behavior, constraints, and design decisions.
- [Controller refactor TODO](docs/controller-refactor-todo.md) — prioritized implementation and testing tasks.

## Working guidance

- This project uses **SDL2**, not SDL3. Use `SDL_GameController*` APIs and SDL2 event names.
- User-facing controller documentation and prompts are **PlayStation-centric** using Western conventions: Cross confirms, Circle cancels, Square is secondary, Triangle is special, and L3/R3 toggles the debug controls overlay. SDL button names should still be used in code.
- Preserve existing keyboard and mouse input paths; controller input should be translated into the game's existing logical input layer where practical.
- Avoid sprinkling SDL controller calls throughout gameplay and UI code. Prefer controller actions and focus/navigation abstractions.
- Treat controller support as optional. The game must continue to start and run when no controller is connected.
- Keep changes incremental and test each UI surface independently.
