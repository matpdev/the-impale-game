## Primary rule (must be followed for ALL user requests)## Primary rule (must be followed before any modification)

**MANDATORY WORKFLOW FOR EVERY REQUEST:**Before making any code or file change — even small ones — present a short todo describing the change (1–2 lines) and wait for explicit confirmation. Do not apply edits until the user approves the todo.

1. **Receive user request** (any type: code change, question, analysis, refactoring, bug fix, feature addition, etc.)## Purpose

2. **Analyze and plan** the response/changes needed

3. **Present a TODO** describing what will be done (1-5 lines, clear and specific)Short, practical guidance for AI coding agents working on the Impale game (the-jumper fork).

4. **Wait for explicit user confirmation** ("yes", "ok", "do it", "proceed", etc.)

5. **Execute ONLY after approval** — never take action without confirmationWork goal: help an agent be productive quickly by calling out the project's build flow, important files, code patterns, and integration points discovered in the repository.

**This applies to:**## Game idea: Impale — quick summary & rules

- Code or file modifications (any size)

- Creating new files or directories- Title: Impale — a physics micro-mission game built with Box2D + raylib.

- Deleting or renaming files- Core mechanic: throw objects (no throw limit) and use physics to impale or otherwise destroy them on hazards (spikes, flame, grenades, scissors). Each level is a short micro-mission with a simple objective (e.g., "Impale 3 objects", "Cut the red object", "Trigger the explosive").

- Refactoring or restructuring code- Micro-mission rules:

- Installing dependencies or packages - Unlimited throws; score or progress determined by mission goal, efficiency, or special interactions (e.g., burn, cut, explode).

- Running commands or builds - Hazards have distinct behaviors: spikes (impale & stick), flame (ignite object, causes damage over time), grenades (area damage), scissors (cut object into pieces).

- Analysis or investigation that requires tool usage - Objects may have simple material properties (mass, breakable, flammable, cuttable).

- ANY action that changes project state - Missions should be short (30–90 seconds) and designed so a single throw can succeed or fail interestingly.

- Success/failure: missions define explicit success conditions (count, state change, or score threshold). Failure occurs when attempts/time run out or a condition is unmet.

**TODO format examples:**

- Code change: "Update `main.cpp` line 42 to fix null pointer check in collision system"## Architecture & important files

- New feature: "Create `input_system.hpp` with keyboard/mouse handling logic (3 functions, ~50 lines)"

- Refactor: "Extract rendering code from `main.cpp` into `render_system.hpp` (move 120 lines)"- `src/main.cpp` — current single-file runtime. Contains initialization, Box2D world setup, asset loads, main loop, input handling (mouse-drag throw), and rendering. This is the first place to read to understand flow.

- Investigation: "Search codebase for usage of `UpdateLogic()` function to verify call patterns"- `xmake.lua` — build and packaging. Pulls packages (`raylib`, `raygui`, `box2d`) and copies assets via `add_configfiles`. Runs `update_includes.lua` in `before_build`.

- Build: "Run `xmake` to compile project and verify no errors"- `update_includes.lua` — helper that reads `compile_commands.json` and updates `.vscode/c_cpp_properties.json` to include xmake package includes. It currently scans paths under the user's home `.xmake/packages/...`.

- `src/assets/` — textures used by the game (`box.png`, `ground.png`).

**DO NOT:**

- Make any changes without presenting TODO first## ECS requirement (must)

- Assume approval from context ("you said earlier", "continuing from before")

- Skip confirmation for "small" or "obvious" changes- All gameplay systems, components, entity creation, memory and lifetime management must use an ECS pattern.

- Present multiple TODOs and execute the first one immediately - Components should be plain data structs (no heavy logic).

  - Systems operate on component arrays or views and are responsible for stepping physics, rendering, input, and mission logic.

**Exception:** Pure informational responses (explaining concepts, answering "what is X?", etc.) don't require TODO, but any action does. - Entity creation/destruction must be centralized (no ad-hoc new/delete scattered across gameplay code).

- Memory: prefer contiguous arrays/pools for components; use stable entity IDs (index + generation) to avoid dangling references.

## Purpose- Migration plan hint: start by extracting Entity, Transform, PhysicsBody, Sprite, and Mission components from `main.cpp` into `includes/components/*` and implement a minimal entity manager in `includes/core/`.

Short, practical guidance for AI coding agents working on the Impale game (the-jumper fork).## Project-specific patterns & conventions

Work goal: help an agent be productive quickly by calling out the project's build flow, important files, code patterns, and integration points discovered in the repository.- Units conversion is explicit: `lengthUnitsPerMeter` and `b2SetLengthUnitsPerMeter(lengthUnitsPerMeter)`. Convert between physics meters and pixel coordinates when creating bodies and rendering.

- Input spawn/throw behavior is implemented via mouse drag in `main.cpp` — replicate this logic inside an Input or Throw system that emits an entity with PhysicsBody and ThrowComponent.

## Game idea: Impale — quick summary & rules- `includes/` contains scaffolding for `components/`, `core/`, `entities/`, `systems/` — use these folders for ECS implementation.

- Title: Impale — a physics micro-mission game built with Box2D + raylib.## Build & run (developer workflow)

- Core mechanic: throw objects (no throw limit) and use physics to impale or otherwise destroy them on hazards (spikes, flame, grenades, scissors). Each level is a short micro-mission with a simple objective (e.g., "Impale 3 objects", "Cut the red object", "Trigger the explosive").

- Micro-mission rules:- Configure + build (Linux):

  - Unlimited throws; score or progress determined by mission goal, efficiency, or special interactions (e.g., burn, cut, explode). ```fish

  - Hazards have distinct behaviors: spikes (impale & stick), flame (ignite object, causes damage over time), grenades (area damage), scissors (cut object into pieces). xmake f -p linux -a x86_64 -m release

  - Objects may have simple material properties (mass, breakable, flammable, cuttable). xmake

  - Missions should be short (30–90 seconds) and designed so a single throw can succeed or fail interestingly. ```

- Success/failure: missions define explicit success conditions (count, state change, or score threshold). Failure occurs when attempts/time run out or a condition is unmet.- Binary: `build/linux/x86_64/release/the-jumper`

- Optional runner: `xmake run the-jumper`

## Architecture & important files- Note: `xmake.lua` runs `update_includes.lua` in `before_build`. Ensure `compile_commands.json` exists (xmake compile-commands plugin) before relying on the update script.

- `src/main.cpp` — main entry point with game initialization and main loop## Integration & dependency notes

- `src/includes/systems/` — game systems (logic_system.hpp, render_system.hpp)

- `src/includes/components/` — ECS components (plain data structs)- Packages are managed by xmake: `raylib`, `raygui`, `box2d`. Add or change dependencies in `xmake.lua`.

- `src/includes/entities/` — entity types and factory functions- Keep `update_includes.lua` free from hardcoded absolute paths — it currently looks for package include paths under a home directory pattern. If you change it, prefer `${workspaceFolder}` style variables or config-driven paths.

- `src/includes/core/` — core utilities (entity_manager.hpp, world_loader.hpp)

- `xmake.lua` — build and packaging. Pulls packages (`raylib`, `raygui`, `box2d`) and copies assets## Guidance for AI agents (operational rules)

- `update_includes.lua` — helper that updates `.vscode/c_cpp_properties.json` with xmake package includes

- `src/assets/` — textures and resources used by the game- Follow the Primary rule at the top: always present a todo for any small change and wait for explicit user confirmation.

- `src/assets/levels/` — TOML level configurations- When proposing code edits, show a minimal, concrete todo (1–3 lines) and a short rationale. Wait for user approval before applying.

- Prefer small, local changes. When extracting ECS, do it incrementally: extract a component or system and run the build/tests before continuing.

## ECS requirement (must)- Avoid modifying build tooling (`xmake.lua`, `update_includes.lua`) unless the user requests it explicitly. If needed, include the change as a separate todo for approval.

- All gameplay systems, components, entity creation, memory and lifetime management must use an ECS pattern.## When to ask for human guidance

  - Components should be plain data structs (no heavy logic).

  - Systems operate on component arrays or views and are responsible for stepping physics, rendering, input, and mission logic.- Major refactors (move core logic out of `main.cpp` into ECS) — present a migration plan as a todo first.

  - Entity creation/destruction must be centralized (no ad-hoc new/delete scattered across gameplay code).- Any change that introduces absolute paths, new global tooling, or breaks the build on CI.

  - Memory: prefer contiguous arrays/pools for components; use stable entity IDs (index + generation) to avoid dangling references.

- Current implementation:---

  - Logic system: `UpdateLogic()` in `logic_system.hpp` handles physics, collision, input

  - Render system: `RenderFrame()` in `render_system.hpp` handles all drawingIf anything above is unclear or you'd like examples for refactoring `main.cpp` into a simple component/system split, tell me which slice to extract and I will produce the changes and a short test harness.

  - Main loop: minimal, just calls systems in sequence

## Primary rule (must be followed before any modification)

## Project-specific patterns & conventions

Before making any code or file change — even small ones — present a short todo describing the change (1–2 lines) and wait for explicit confirmation. Do not apply edits until the user approves the todo.

- Units conversion is explicit: `lengthUnitsPerMeter` and `b2SetLengthUnitsPerMeter(lengthUnitsPerMeter)`. Convert between physics meters and pixel coordinates when creating bodies and rendering.

- Texture loading: uses `TextureCache` in main.cpp for deduplication; textures loaded from TOML paths## Purpose

- Entity creation: use factory functions in `entities/factory.hpp` (makeBoxEntity, makeSpikeEntity, etc.)

- Systems use context structs: `LogicContext` and `RenderContext` hold all needed stateShort, practical guidance for AI coding agents working on the Impale game (the-jumper fork).

- TOML configuration: levels defined in `src/assets/levels/*.toml` with texture paths per entity

Work goal: help an agent be productive quickly by calling out the project's build flow, important files, code patterns, and integration points discovered in the repository.

## Build & run (developer workflow)

## Game idea: Impale — quick summary & rules

- Configure + build (Linux):

  ````fish- Title: Impale — a physics micro-mission game built with Box2D + raylib.

  xmake f -p linux -a x86_64 -m release- Core mechanic: throw objects (no throw limit) and use physics to impale or otherwise destroy them on hazards (spikes, flame, grenades, scissors). Each level is a short micro-mission with a simple objective (e.g., "Impale 3 objects", "Cut the red object", "Trigger the explosive").

  xmake- Micro-mission rules:

  ```  - Unlimited throws; score or progress determined by mission goal, efficiency, or special interactions (e.g., burn, cut, explode).

  ````

- Binary: `build/linux/x86_64/release/the-jumper` - Hazards have distinct behaviors: spikes (impale & stick), flame (ignite object, causes damage over time), grenades (area damage), scissors (cut object into pieces).

- Optional runner: `xmake run the-jumper` - Objects may have simple material properties (mass, breakable, flammable, cuttable).

- Note: `xmake.lua` runs `update_includes.lua` in `before_build`. Ensure `compile_commands.json` exists (xmake compile-commands plugin) before relying on the update script. - Missions should be short (30–90 seconds) and designed so a single throw can succeed or fail interestingly.

- Success/failure: missions define explicit success conditions (count, state change, or score threshold). Failure occurs when attempts/time run out or a condition is unmet.

## Integration & dependency notes

## Architecture & important files

- Packages are managed by xmake: `raylib`, `raygui`, `box2d`. Add or change dependencies in `xmake.lua`.

- Keep `update_includes.lua` free from hardcoded absolute paths — it currently looks for package include paths under a home directory pattern. If you change it, prefer `${workspaceFolder}` style variables or config-driven paths.- `src/main.cpp` — current single-file runtime. Contains initialization, Box2D world setup, asset loads, main loop, input handling (mouse-drag throw), and rendering. This is the first place to read to understand flow.

- `xmake.lua` — build and packaging. Pulls packages (`raylib`, `raygui`, `box2d`) and copies assets via `add_configfiles`. Runs `update_includes.lua` in `before_build`.

## Guidance for AI agents (operational rules)- `update_includes.lua` — helper that reads `compile_commands.json` and updates `.vscode/c_cpp_properties.json` to include xmake package includes. It currently scans paths under the user's home `.xmake/packages/...`.

- `src/assets/` — textures used by the game (`box.png`, `ground.png`).

- **Always follow the Primary rule**: present TODO for any request, wait for confirmation

- When proposing code edits, show a minimal, concrete TODO (1–5 lines) with clear intent. Wait for user approval before applying.## ECS requirement (must)

- Prefer small, incremental changes. When adding features or refactoring, do it step-by-step and verify builds between steps.

- Avoid modifying build tooling (`xmake.lua`, `update_includes.lua`) unless the user requests it explicitly. If needed, include the change as a separate TODO for approval.- All gameplay systems, components, entity creation, memory and lifetime management must use an ECS pattern.

- Use existing patterns: context structs for systems, factory functions for entities, TOML for configuration - Components should be plain data structs (no heavy logic).

- Test after changes: run `xmake` to verify compilation, `xmake run` to verify runtime behavior - Systems operate on component arrays or views and are responsible for stepping physics, rendering, input, and mission logic.

  - Entity creation/destruction must be centralized (no ad-hoc new/delete scattered across gameplay code).

## When to ask for human guidance - Memory: prefer contiguous arrays/pools for components; use stable entity IDs (index + generation) to avoid dangling references.

- Migration plan hint: start by extracting Entity, Transform, PhysicsBody, Sprite, and Mission components from `main.cpp` into `includes/components/*` and implement a minimal entity manager in `includes/core/`.

- Major architectural changes (new systems, significant refactoring)

- Changes that affect build configuration or CI## Project-specific patterns & conventions

- Unclear requirements or ambiguous requests

- Any change that introduces absolute paths, new global tooling, or breaks existing patterns- Units conversion is explicit: `lengthUnitsPerMeter` and `b2SetLengthUnitsPerMeter(lengthUnitsPerMeter)`. Convert between physics meters and pixel coordinates when creating bodies and rendering.

- Input spawn/throw behavior is implemented via mouse drag in `main.cpp` — replicate this logic inside an Input or Throw system that emits an entity with PhysicsBody and ThrowComponent.

---- `includes/` contains scaffolding for `components/`, `core/`, `entities/`, `systems/` — use these folders for ECS implementation.

**Remember:** No matter how simple the request seems, always present a TODO and wait for explicit confirmation before taking any action.## Build & run (developer workflow)

- Configure + build (Linux):
  ```fish
  xmake f -p linux -a x86_64 -m release
  xmake
  ```
- Binary: `build/linux/x86_64/release/the-jumper`
- Optional runner: `xmake run the-jumper`
- Note: `xmake.lua` runs `update_includes.lua` in `before_build`. Ensure `compile_commands.json` exists (xmake compile-commands plugin) before relying on the update script.

## Integration & dependency notes

- Packages are managed by xmake: `raylib`, `raygui`, `box2d`. Add or change dependencies in `xmake.lua`.
- Keep `update_includes.lua` free from hardcoded absolute paths — it currently looks for package include paths under a home directory pattern. If you change it, prefer `${workspaceFolder}` style variables or config-driven paths.

## Guidance for AI agents (operational rules)

- Follow the Primary rule at the top: always present a todo for any small change and wait for explicit user confirmation.
- When proposing code edits, show a minimal, concrete todo (1–3 lines) and a short rationale. Wait for user approval before applying.
- Prefer small, local changes. When extracting ECS, do it incrementally: extract a component or system and run the build/tests before continuing.
- Avoid modifying build tooling (`xmake.lua`, `update_includes.lua`) unless the user requests it explicitly. If needed, include the change as a separate todo for approval.

## When to ask for human guidance

- Major refactors (move core logic out of `main.cpp` into ECS) — present a migration plan as a todo first.
- Any change that introduces absolute paths, new global tooling, or breaks the build on CI.

---

If anything above is unclear or you'd like examples for refactoring `main.cpp` into a simple component/system split, tell me which slice to extract and I will produce the changes and a short test harness.
