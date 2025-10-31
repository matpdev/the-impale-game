## Primary rule (must be followed before any modification)

Before making any code or file change — even small ones — present a short todo describing the change (1–2 lines) and wait for explicit confirmation. Do not apply edits until the user approves the todo.

## Purpose

Short, practical guidance for AI coding agents working on the Impale game (the-jumper fork).

Work goal: help an agent be productive quickly by calling out the project's build flow, important files, code patterns, and integration points discovered in the repository.

## Game idea: Impale — quick summary & rules

- Title: Impale — a physics micro-mission game built with Box2D + raylib.
- Core mechanic: throw objects (no throw limit) and use physics to impale or otherwise destroy them on hazards (spikes, flame, grenades, scissors). Each level is a short micro-mission with a simple objective (e.g., "Impale 3 objects", "Cut the red object", "Trigger the explosive").
- Micro-mission rules:
  - Unlimited throws; score or progress determined by mission goal, efficiency, or special interactions (e.g., burn, cut, explode).
  - Hazards have distinct behaviors: spikes (impale & stick), flame (ignite object, causes damage over time), grenades (area damage), scissors (cut object into pieces).
  - Objects may have simple material properties (mass, breakable, flammable, cuttable).
  - Missions should be short (30–90 seconds) and designed so a single throw can succeed or fail interestingly.
- Success/failure: missions define explicit success conditions (count, state change, or score threshold). Failure occurs when attempts/time run out or a condition is unmet.

## Architecture & important files

- `src/main.cpp` — current single-file runtime. Contains initialization, Box2D world setup, asset loads, main loop, input handling (mouse-drag throw), and rendering. This is the first place to read to understand flow.
- `xmake.lua` — build and packaging. Pulls packages (`raylib`, `raygui`, `box2d`) and copies assets via `add_configfiles`. Runs `update_includes.lua` in `before_build`.
- `update_includes.lua` — helper that reads `compile_commands.json` and updates `.vscode/c_cpp_properties.json` to include xmake package includes. It currently scans paths under the user's home `.xmake/packages/...`.
- `src/assets/` — textures used by the game (`box.png`, `ground.png`).

## ECS requirement (must)

- All gameplay systems, components, entity creation, memory and lifetime management must use an ECS pattern.
  - Components should be plain data structs (no heavy logic).
  - Systems operate on component arrays or views and are responsible for stepping physics, rendering, input, and mission logic.
  - Entity creation/destruction must be centralized (no ad-hoc new/delete scattered across gameplay code).
  - Memory: prefer contiguous arrays/pools for components; use stable entity IDs (index + generation) to avoid dangling references.
- Migration plan hint: start by extracting Entity, Transform, PhysicsBody, Sprite, and Mission components from `main.cpp` into `includes/components/*` and implement a minimal entity manager in `includes/core/`.

## Project-specific patterns & conventions

- Units conversion is explicit: `lengthUnitsPerMeter` and `b2SetLengthUnitsPerMeter(lengthUnitsPerMeter)`. Convert between physics meters and pixel coordinates when creating bodies and rendering.
- Input spawn/throw behavior is implemented via mouse drag in `main.cpp` — replicate this logic inside an Input or Throw system that emits an entity with PhysicsBody and ThrowComponent.
- `includes/` contains scaffolding for `components/`, `core/`, `entities/`, `systems/` — use these folders for ECS implementation.

## Build & run (developer workflow)

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

## Primary rule (must be followed before any modification)

Before making any code or file change — even small ones — present a short todo describing the change (1–2 lines) and wait for explicit confirmation. Do not apply edits until the user approves the todo.

## Purpose

Short, practical guidance for AI coding agents working on the Impale game (the-jumper fork).

Work goal: help an agent be productive quickly by calling out the project's build flow, important files, code patterns, and integration points discovered in the repository.

## Game idea: Impale — quick summary & rules

- Title: Impale — a physics micro-mission game built with Box2D + raylib.
- Core mechanic: throw objects (no throw limit) and use physics to impale or otherwise destroy them on hazards (spikes, flame, grenades, scissors). Each level is a short micro-mission with a simple objective (e.g., "Impale 3 objects", "Cut the red object", "Trigger the explosive").
- Micro-mission rules:
  - Unlimited throws; score or progress determined by mission goal, efficiency, or special interactions (e.g., burn, cut, explode).
  - Hazards have distinct behaviors: spikes (impale & stick), flame (ignite object, causes damage over time), grenades (area damage), scissors (cut object into pieces).
  - Objects may have simple material properties (mass, breakable, flammable, cuttable).
  - Missions should be short (30–90 seconds) and designed so a single throw can succeed or fail interestingly.
- Success/failure: missions define explicit success conditions (count, state change, or score threshold). Failure occurs when attempts/time run out or a condition is unmet.

## Architecture & important files

- `src/main.cpp` — current single-file runtime. Contains initialization, Box2D world setup, asset loads, main loop, input handling (mouse-drag throw), and rendering. This is the first place to read to understand flow.
- `xmake.lua` — build and packaging. Pulls packages (`raylib`, `raygui`, `box2d`) and copies assets via `add_configfiles`. Runs `update_includes.lua` in `before_build`.
- `update_includes.lua` — helper that reads `compile_commands.json` and updates `.vscode/c_cpp_properties.json` to include xmake package includes. It currently scans paths under the user's home `.xmake/packages/...`.
- `src/assets/` — textures used by the game (`box.png`, `ground.png`).

## ECS requirement (must)

- All gameplay systems, components, entity creation, memory and lifetime management must use an ECS pattern.
  - Components should be plain data structs (no heavy logic).
  - Systems operate on component arrays or views and are responsible for stepping physics, rendering, input, and mission logic.
  - Entity creation/destruction must be centralized (no ad-hoc new/delete scattered across gameplay code).
  - Memory: prefer contiguous arrays/pools for components; use stable entity IDs (index + generation) to avoid dangling references.
- Migration plan hint: start by extracting Entity, Transform, PhysicsBody, Sprite, and Mission components from `main.cpp` into `includes/components/*` and implement a minimal entity manager in `includes/core/`.

## Project-specific patterns & conventions

- Units conversion is explicit: `lengthUnitsPerMeter` and `b2SetLengthUnitsPerMeter(lengthUnitsPerMeter)`. Convert between physics meters and pixel coordinates when creating bodies and rendering.
- Input spawn/throw behavior is implemented via mouse drag in `main.cpp` — replicate this logic inside an Input or Throw system that emits an entity with PhysicsBody and ThrowComponent.
- `includes/` contains scaffolding for `components/`, `core/`, `entities/`, `systems/` — use these folders for ECS implementation.

## Build & run (developer workflow)

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

If anything above is unclear or you'd like examples for refactoring `main.cpp` into a simple component/system split, tell me which slice to extract and I will produce the changes and a short test harness.
