# Impale - Game Architecture Documentation

## Table of Contents
1. [Project Overview](#project-overview)
2. [Technology Stack](#technology-stack)
3. [Build System](#build-system)
4. [Architecture Overview](#architecture-overview)
5. [Directory Structure](#directory-structure)
6. [Core Systems](#core-systems)
7. [Components](#components)
8. [Entity Types](#entity-types)
9. [Asset Management](#asset-management)
10. [Platform-Specific Details](#platform-specific-details)

---

## Project Overview

**Impale** is a physics-based micro-mission game where players throw objects to impale, destroy, or interact with hazards using realistic 2D physics. The game is built with a focus on short, replayable missions (30-90 seconds) with varied objectives.

### Core Mechanics
- **Unlimited Throws**: No throw limit; success is based on mission objectives
- **Physics Interactions**: Objects interact realistically with spikes, saws, chains, and other hazards
- **Micro-Missions**: Short objectives like "Impale 3 objects", "Cut the red object", or "Trigger the explosive"
- **Hazard Types**:
  - **Spikes**: Impale and stick objects via physics joints
  - **Saws**: Rotating hazards that cut/damage objects
  - **Chains**: Hanging hooks that capture objects with rope physics
  - **Future**: Flames, grenades, scissors, and more

### Game Rules
- Objects have material properties (mass, breakable, flammable, cuttable)
- Success/failure determined by mission-specific conditions
- Efficiency and creativity rewarded through special interactions

---

## Technology Stack

### Core Libraries

#### raylib (5.5)
- **Purpose**: Rendering, windowing, input handling, and texture management
- **Usage**: Main game loop, sprite rendering, UI, and cross-platform window management
- **Platform Support**: Native (Linux/Windows/macOS) and Web (via Emscripten)

#### Box2D v3.1.1
- **Purpose**: 2D physics simulation
- **Usage**: 
  - Rigidbody dynamics
  - Collision detection
  - Joint constraints (revolute, distance)
  - Physics material properties (friction, restitution, density)
- **Features Used**:
  - Dynamic/Static/Kinematic bodies
  - Polygon shapes
  - Revolute joints (for pendulum-like impaling)
  - Distance joints (for rope/chain physics)

#### toml11 (v4.4.0)
- **Purpose**: Configuration and level data parsing
- **Usage**: Load level definitions from TOML files
- **Format**: Declarative level structure with entities, properties, and positions

#### raygui (4.0)
- **Purpose**: Immediate-mode GUI toolkit
- **Usage**: Debug UI, future in-game menus
- **Note**: Not available in WASM builds (excluded from web target)

---

## Build System

### xmake

The project uses **xmake** as its build system and package manager.

#### Targets

##### 1. Native Build (`the-impale-game`)
```fish
xmake f -p linux -a x86_64 -m release
xmake build the-impale-game
```

**Output**: `build/linux/x86_64/release/the-impale-game`

**Features**:
- Full raygui support
- Native window management
- Direct file system access
- Optimized for desktop performance

##### 2. Web Build (`the-impale-game-web`)
```fish
xmake f -p wasm -a wasm32 --toolchain=emcc -m release
xmake build the-impale-game-web
```

**Output**: `build/wasm/release/`
- `the-impale-game-web.html` - Main HTML page
- `the-impale-game-web.js` - JavaScript loader
- `the-impale-game-web.wasm` - Compiled WebAssembly binary
- `the-impale-game-web.data` - Preloaded assets

**Features**:
- Emscripten-based WASM compilation
- Virtual filesystem for assets (`/assets/`)
- No raygui (unsupported on WASM)
- Browser-compatible input/rendering

#### Package Management
- Packages automatically installed via xmake-repo
- Platform-specific configurations:
  - **WASM**: raylib with `platform = "Web"`
  - **Native**: Full package set including raygui

#### Build Hooks
- **before_build**: Runs `update_includes.lua` to sync VSCode IntelliSense with xmake package includes

---

## Architecture Overview

### Entity-Component-System (ECS) Pattern

The game follows a **data-oriented ECS architecture** for managing game entities and their behavior.

#### Design Principles
1. **Components = Data**: Plain structs with no logic
2. **Systems = Logic**: Operate on component arrays
3. **Entities = IDs**: Stable identifiers with generation counters
4. **Centralized Management**: EntityManager handles creation/destruction

#### Benefits
- **Performance**: Cache-friendly memory layout
- **Modularity**: Easy to add/remove components
- **Maintainability**: Clear separation of data and logic
- **Scalability**: Efficient iteration over large entity counts

### Main Loop Flow

```
Initialize → Load Level → Game Loop → Cleanup
                            ↓
              ┌─────────────┴──────────────┐
              ↓                            ↓
         UpdateLogic()                RenderFrame()
              ↓                            ↓
    • Physics Step                 • Clear Screen
    • Collision Detection          • Draw Entities
    • Input Handling               • Debug Overlay
    • Entity Updates               • UI Rendering
```

---

## Directory Structure

```
/home/mistergrow/Projects/the-jumper/
├── xmake.lua                    # Build configuration
├── src/
│   ├── main.cpp                 # Entry point, main loop
│   ├── core/
│   │   └── world_loader.cpp     # TOML level parsing implementation
│   ├── includes/
│   │   ├── components/          # ECS component definitions
│   │   │   ├── impaled.hpp      # Impalement state (frozen, joint)
│   │   │   ├── physics_body.hpp # Box2D body wrapper
│   │   │   ├── physics_material.hpp  # Material properties
│   │   │   ├── script.hpp       # Per-entity behavior hooks
│   │   │   ├── spike_properties.hpp  # Spike type & config
│   │   │   ├── sprite.hpp       # Texture rendering data
│   │   │   ├── transform.hpp    # Position & extent
│   │   │   └── visual_style.hpp # Color, roundness, texture flags
│   │   ├── core/
│   │   │   ├── entity_manager.hpp    # Entity ID lifecycle
│   │   │   └── world_loader.hpp      # Level loading interface
│   │   ├── entities/
│   │   │   ├── factory.hpp      # Entity creation functions
│   │   │   ├── player.hpp       # Player entity type (future)
│   │   │   └── types.hpp        # GameEntity struct definition
│   │   └── systems/
│   │       ├── logic_system.hpp # Physics, collision, input
│   │       └── render_system.hpp # Drawing, debug overlays
│   ├── assets/
│   │   ├── block.png            # Block texture
│   │   ├── box.png              # Box/projectile texture
│   │   ├── ground.png           # Ground/platform texture
│   │   └── levels/
│   │       └── demo.toml        # Demo level configuration
│   └── tools/
│       └── update_includes.lua  # VSCode include path updater
└── build/                       # Output directory (gitignored)
    ├── linux/x86_64/release/    # Native build
    └── wasm/release/            # Web build
```

### File Responsibilities

| File | Purpose |
|------|---------|
| `main.cpp` | Game initialization, main loop, platform-specific asset paths |
| `world_loader.cpp` | Parse TOML files and construct entities via factory |
| `entity_manager.hpp` | Stable entity ID generation with index+generation pattern |
| `factory.hpp` | Entity creation with component initialization |
| `logic_system.hpp` | Game state updates, physics stepping, collision handling |
| `render_system.hpp` | Drawing entities, debug wireframes, UI overlays |
| `types.hpp` | GameEntity struct aggregating all components |

---

## Core Systems

### 1. Logic System (`logic_system.hpp`)

**Responsibility**: Update game state each frame

#### LogicContext
```cpp
struct LogicContext {
    b2WorldId worldId;              // Box2D world
    float lengthUnitsPerMeter;      // Pixel-to-meter conversion
    vector<GameEntity>& boxes;      // Dynamic throwable objects
    vector<GameEntity>& obstacles;  // Static platforms/walls
    vector<GameEntity>& spikes;     // Hazards
    vector<GameEntity>& throwers;   // Launcher entities
    EntityManager& entityManager;
    Texture& boxTexture;
    b2Polygon& boxPolygon;
    b2Vec2& boxExtent;
    bool isPaused;
};
```

#### UpdateLogic() Flow
1. **Physics Step**: `b2World_Step(worldId, deltaTime, 4)`
2. **Collision Detection**: Check box-spike overlaps
3. **Attachment Logic**:
   - **Normal Spikes**: Create revolute joint at impact point
   - **Saws**: Revolute joint for spinning attachment
   - **Chains**: Distance joint from hook to box (rope behavior)
4. **Thrower Input**:
   - Mouse aim calculation
   - Charge power on left-click hold
   - Spawn projectile on release with impulse
5. **Per-Entity Updates**: Call `script.update()` for custom behavior

#### Collision & Attachment
- **Detection**: Simple radius-based distance check
- **Joint Types**:
  - `b2RevoluteJoint`: Pendulum swing on normal spikes/saws
  - `b2DistanceJoint`: Rope constraint for chains
- **Frozen State**: Marks entity as captured, prevents re-attachment

---

### 2. Render System (`render_system.hpp`)

**Responsibility**: Draw all visual elements

#### RenderContext
```cpp
struct RenderContext {
    int screenWidth, screenHeight;
    float lengthUnitsPerMeter;
    vector<GameEntity>& boxes;
    vector<GameEntity>& obstacles;
    vector<GameEntity>& spikes;
    vector<GameEntity>& throwers;
    bool showDebugWireframe;
    DebugRope* debugRope;
};
```

#### RenderFrame() Flow
1. **Clear Screen**: `ClearBackground(DARKGRAY)`
2. **Draw Title**: Static text overlay
3. **Entity Rendering**: Call `script.render()` for each entity
4. **Debug Wireframe** (if enabled):
   - Entity bounding boxes
   - Physics body centers
   - Chain/rope visualizations
   - Entity counts
5. **UI Overlays**: Instructions, debug info

#### DrawSprite() Helper
- Converts Box2D position (meters) to raylib screen space (pixels)
- Applies texture with rotation from physics body
- Supports solid color fallback if `useTexture = false`

#### Custom Renderers
- **ObstacleRender**: Textured rectangle with stretch
- **SpikeRender**: Type-specific drawing:
  - Normal: Textured square
  - Saw: Circle with rotating teeth
  - Chain: Rope line + hook rectangle
- **ThrowerRender**: Aim line + power indicator

---

## Components

Components are **plain data structs** with no methods (except constructors/defaults).

### PhysicsBody (`physics_body.hpp`)
```cpp
struct PhysicsBody {
    b2BodyId id;  // Box2D body handle
};
```
**Purpose**: Reference to the physics simulation body

---

### Transform (`transform.hpp`)
```cpp
struct SpriteTransform {
    b2Vec2 extent;  // Half-width, half-height in pixels
};
```
**Purpose**: Visual size of the entity (decoupled from physics shape)

---

### Sprite (`sprite.hpp`)
```cpp
struct Sprite {
    Texture texture;  // raylib texture handle
};
```
**Purpose**: Visual representation asset

---

### Impaled (`impaled.hpp`)
```cpp
struct ImpaledState {
    bool frozen;
    b2JointId jointId;
    bool hasJoint() const;
};
```
**Purpose**: Tracks if entity is attached to a spike via joint

---

### PhysicsMaterial (`physics_material.hpp`)
```cpp
struct PhysicsMaterial {
    float density;
    float friction;
    float restitution;
    float linearDamping;
    float angularDamping;
    bool affectedByGravity;
};
```
**Purpose**: Defines physical behavior (mass, bounciness, drag)

---

### VisualStyle (`visual_style.hpp`)
```cpp
struct VisualStyle {
    Color color;
    float roundness;
    bool useTexture;
};
```
**Purpose**: Appearance customization (tint, roundness, texture toggle)

---

### SpikeProperties (`spike_properties.hpp`)
```cpp
enum class SpikeType { NORMAL, CHAIN, SAW };

struct SpikeProperties {
    SpikeType type;
    float rotationSpeed;  // for SAW
    float chainLength;    // for CHAIN
    // Chain tuning params...
};
```
**Purpose**: Spike-specific configuration and behavior

---

### Script (`script.hpp`)
```cpp
struct Script {
    void (*update)(GameEntity&, float dt);
    void (*render)(const GameEntity&, float unitsPerMeter);
    void* user;  // Custom context data
    void (*freeFn)(void*);
};
```
**Purpose**: Per-entity behavior hooks and custom data

---

## Entity Types

### GameEntity (`types.hpp`)
```cpp
struct GameEntity {
    EntityId id;
    PhysicsBody body;
    SpriteTransform transform;
    Sprite sprite;
    Script script;
    ImpaledState impaled;
    PhysicsMaterial physics;
    VisualStyle visual;
    SpikeProperties spikeProps;
};
```
**Aggregates all components** into a single struct for convenience.

---

### Factory Functions (`factory.hpp`)

#### makeGroundEntity()
- **Type**: Static body
- **Use**: Immovable platforms, walls
- **Physics**: No gravity, infinite mass

#### makeBoxEntity()
- **Type**: Dynamic body
- **Use**: Throwable projectiles
- **Physics**: Custom material, affected by gravity
- **Visual**: Custom color/texture

#### makeObstacleEntity()
- **Type**: Static body
- **Use**: Variable-sized platforms
- **Physics**: Custom dimensions via extent
- **Render**: Custom texture stretch

#### makeSpikeEntity()
- **Type**: Dynamic body (but heavily dampened)
- **Use**: Hazards that capture objects
- **Physics**: 
  - Very high density (10000) to resist movement
  - Zero gravity
  - High damping (100) to prevent drift
- **Special**:
  - **CHAIN**: Creates rope (distance joint) + hook body
  - **SAW**: Rotating visual via `SpikeUpdate()`

#### makeThrowerEntity()
- **Type**: Static sensor body
- **Use**: Player-controlled launcher
- **Context**: `ThrowerContext` holds aim, charge, projectile list
- **Render**: Aim line + power indicator

---

## Asset Management

### Texture Cache (`main.cpp`)
```cpp
struct TextureCache {
    map<string, Texture> textures;
    Texture load(const string& path);
    void unloadAll();
};
```
**Purpose**: Avoid redundant texture loading, centralized cleanup

---

### TOML Level Format (`demo.toml`)

#### Structure
```toml
[thrower]
x = 600
y = 800
power = 250.0
impulseMultiplier = 18.0
texture = "box.png"

[[obstacles]]
x = 300
y = 980
w = 500
h = 40
color = [139, 69, 19]
texture = "ground.png"

[[spikes]]
x = 1300
y = 820
r = 24
type = "normal"
color = [255, 0, 0]
texture = "box.png"
```

#### Supported Fields
- **Position**: `x`, `y` (pixels)
- **Size**: `w`, `h` (obstacles), `r` (spikes radius)
- **Visual**: `color` (RGBA array), `texture` (path), `roundness`
- **Physics**: `density`, `friction`, `restitution`, `linearDamping`, `angularDamping`, `gravity`
- **Spike**: `type`, `rotationSpeed`, `chainLength`, `linkLengthPx`, etc.

---

### World Loader (`world_loader.cpp`)

#### BuildContext
```cpp
struct BuildContext {
    EntityManager& em;
    b2WorldId world;
    float unitsPerMeter;
    Texture groundTexture, boxTexture;
    b2Polygon groundPolygon, boxPolygon;
    b2Vec2 groundExtent, boxExtent;
    vector<GameEntity>& grounds, boxes, obstacles, spikes, throwers;
    function<Texture(const string&)> textureLoader;
};
```

#### LoadScenarioFromToml()
1. Parse TOML file with `toml11`
2. Iterate arrays: `obstacles`, `spikes`, `thrower`
3. Extract properties (position, size, material, visual)
4. Load textures via `textureLoader` lambda
5. Call factory functions to create entities
6. Push to context's entity vectors

---

## Platform-Specific Details

### Asset Path Handling

#### Native Builds
- Assets loaded from relative paths: `"ground.png"`, `"levels/demo.toml"`
- Working directory: `build/linux/x86_64/release/`
- Assets copied to build dir via `add_configfiles()` in xmake.lua

#### WASM Builds
- Assets preloaded into Emscripten virtual filesystem at `/assets/`
- Paths prefixed with `/assets/`: `"/assets/ground.png"`, `"/assets/levels/demo.toml"`
- Macro-based conversion:
  ```cpp
  #ifdef __EMSCRIPTEN__
      #define ASSET_PATH(path) ("/assets/" path)
  #else
      #define ASSET_PATH(path) (path)
  #endif
  ```
- Texture loader lambda auto-prepends `/assets/` for WASM

---

### Build Differences

| Feature | Native | WASM |
|---------|--------|------|
| raygui | ✅ Included | ❌ Excluded |
| File I/O | Direct filesystem | Virtual FS (preloaded) |
| Window | OS-native | Browser canvas |
| Input | Direct raylib | Browser events → raylib |
| Performance | CPU native | JIT/AOT WASM |
| Deploy | Single binary | HTML + JS + WASM + .data |

---

### Running the Game

#### Native
```fish
cd build/linux/x86_64/release
./the-impale-game
```

#### Web
```fish
cd build/wasm/release
python -m http.server 8000
# Open: http://localhost:8000/the-impale-game-web.html
```

---

## Development Workflow

### Adding a New Component
1. Create header in `src/includes/components/`
2. Define plain struct with public fields
3. Add to `GameEntity` in `types.hpp`
4. Update factory functions if needed
5. Handle in systems (logic/render)

### Adding a New Entity Type
1. Create factory function in `factory.hpp`
2. Define custom `update()` and `render()` if needed
3. Add to level TOML schema in `world_loader.cpp`
4. Update `BuildContext` if new entity category

### Adding a New Hazard Type
1. Extend `SpikeType` enum
2. Add properties to `SpikeProperties`
3. Update `SpikeUpdate()` for behavior
4. Update `SpikeRender()` for visuals
5. Add collision logic in `UpdateLogic()`
6. Add TOML parsing in `parseSpikeProperties()`

### Debugging
- **Toggle Debug Wireframe**: Press `D` during gameplay
- **Pause Simulation**: Press `P`
- **Entity Counts**: Displayed in debug overlay
- **Console Output**: Texture loading, TOML parsing errors

---

## Future Enhancements

### Planned Features
- **More Hazards**: Flames (ignite objects), grenades (area damage), scissors (cut objects)
- **Breakable Objects**: Objects split/fragment on impact
- **Mission System**: Objectives, scoring, time limits, failure conditions
- **Level Editor**: In-game TOML editing with live reload
- **Sound Effects**: Impact, impalement, success/failure audio
- **Particle Effects**: Sparks, debris, blood splatters
- **Multiplayer**: Shared physics world, competitive scoring

### Technical Debt
- **Component Arrays**: Migrate from single `GameEntity` struct to separate component vectors
- **Memory Pools**: Preallocate entity/component memory for performance
- **Spatial Partitioning**: Optimize collision detection with quadtree/grid
- **Asset Hot-Reload**: Reload textures/levels without restart
- **Save System**: Persist mission progress, unlocks, settings

---

## Contributing

### Code Style
- **Components**: Plain structs, PascalCase names
- **Systems**: Free functions, `UpdateX()` / `RenderX()` naming
- **Factories**: `makeXEntity()` pattern
- **Helpers**: Inline functions in headers where appropriate

### Testing
- **Unit Tests**: Not yet implemented (planned: Catch2)
- **Manual Testing**: Build both native + WASM, verify gameplay

### Pull Request Guidelines
1. Follow ECS pattern (data in components, logic in systems)
2. Update this documentation if architecture changes
3. Test on both native and WASM platforms
4. Include TOML level examples for new features

---

## License & Credits

- **raylib**: zlib/libpng license
- **Box2D**: MIT license
- **toml11**: MIT license
- **raygui**: zlib/libpng license

---

**Last Updated**: November 1, 2025
**Game Version**: Early Development (pre-alpha)
**Target Platforms**: Linux (native), Windows (native), macOS (native), Web (WASM)
