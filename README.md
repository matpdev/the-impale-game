# Impale 🎯

A physics-based micro-mission game where you throw objects to impale, destroy, and interact with hazards using realistic 2D physics.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS%20%7C%20Web-lightgrey)
![Build](https://img.shields.io/badge/build-xmake-green)

## ✨ Features

- **Physics-Driven Gameplay**: Powered by Box2D v3.1.1 for realistic 2D physics
- **Multiple Hazard Types**:
  - 🔺 **Spikes**: Impale objects with revolute joint physics
  - 🪚 **Rotating Saws**: Dynamic hazards with spinning visuals
  - ⛓️ **Chain Hooks**: Rope physics using distance joints
- **Unlimited Throws**: No limits - success based on creativity and objectives
- **Cross-Platform**: Native desktop builds and browser (WebAssembly)
- **Level System**: TOML-based declarative level configuration
- **Debug Mode**: Visualize physics bodies, joints, and collision shapes

## 🎮 Gameplay

Throw objects (boxes) to:
- Impale them on spikes
- Attach them to chain hooks
- Interact with rotating saws
- Complete mission objectives (future)

**Controls**:
- **Mouse Drag**: Aim and charge thrower
- **Left Click**: Fire projectile
- **P**: Pause/Unpause simulation
- **D**: Toggle debug wireframe

## 🚀 Quick Start

### Prerequisites

#### Native Build
- C++17 compiler (GCC, Clang, MSVC)
- [xmake](https://xmake.io/) build system
- raylib, Box2D, toml11 (auto-installed via xmake)

#### Web Build (WASM)
- All native prerequisites
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)

**Install Emscripten on Arch Linux:**
```bash
# Option 1: AUR (easiest)
yay -S emscripten

# Option 2: Manual
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ~/emsdk/emsdk_env.fish  # for Fish shell
```

### Build & Run

#### Quick Build Script (Recommended)

Use the provided `build.fish` script for easy platform switching:

```fish
# Native build and run
./build.fish native --run

# Web build and serve
./build.fish web --serve

# Clean rebuild
./build.fish native --clean

# Debug mode
./build.fish native --debug

# Show all options
./build.fish --help
```

#### Manual Build (Advanced)

##### Native (Linux)
```bash
# Configure
xmake f -p linux -a x86_64 -m release

# Build
xmake build the-impale-game

# Run
xmake run the-impale-game
# or
./build/linux/x86_64/release/the-impale-game
```

##### Web (WASM)
```bash
# Configure
xmake f -p wasm -a wasm32 --toolchain=emcc -m release

# Build
xmake build the-impale-game-web

# Serve locally
cd build/wasm/release
python -m http.server 8000

# Open browser to: http://localhost:8000/the-impale-game-web.html
```

## 📁 Project Structure

```
the-jumper/
├── src/
│   ├── main.cpp                 # Entry point, game loop
│   ├── core/
│   │   └── world_loader.cpp     # TOML level parsing
│   ├── includes/
│   │   ├── components/          # ECS components (data)
│   │   ├── core/                # Entity manager, loaders
│   │   ├── entities/            # Entity types and factories
│   │   └── systems/             # Game logic and rendering
│   └── assets/
│       ├── *.png                # Textures
│       └── levels/*.toml        # Level configurations
├── xmake.lua                    # Build configuration
└── ARCHITECTURE.md              # Detailed documentation
```

## 🛠️ Technology Stack

| Component | Library | Version |
|-----------|---------|---------|
| **Rendering** | [raylib](https://www.raylib.com/) | 5.5 |
| **Physics** | [Box2D](https://box2d.org/) | v3.1.1 |
| **Config** | [toml11](https://github.com/ToruNiina/toml11) | v4.4.0 |
| **GUI** | [raygui](https://github.com/raysan5/raygui) | 4.0 (native only) |
| **Build** | [xmake](https://xmake.io/) | Latest |

## 📖 Documentation

For detailed architecture, component descriptions, and development guidelines, see:
- **[ARCHITECTURE.md](./ARCHITECTURE.md)** - Complete technical documentation
- **[.github/copilot-instructions.md](./.github/copilot-instructions.md)** - Development workflow and AI agent guidelines

## 🎯 Roadmap

### Current (Pre-Alpha)
- [x] Basic physics simulation
- [x] Spike, saw, and chain hazards
- [x] TOML level loading
- [x] Native and WASM builds
- [x] Debug visualization

### Planned (Alpha)
- [ ] Mission system with objectives
- [ ] More hazard types (flames, grenades, scissors)
- [ ] Breakable/cuttable objects
- [ ] Sound effects and particles
- [ ] Level progression and scoring
- [ ] In-game level editor

### Future (Beta+)
- [ ] Multiple mission packs
- [ ] Steam Workshop integration
- [ ] Multiplayer support
- [ ] Leaderboards and replays

## 🤝 Contributing

Contributions are welcome! Please:

1. Read [ARCHITECTURE.md](./ARCHITECTURE.md) to understand the codebase
2. Follow the ECS pattern (components = data, systems = logic)
3. Test on both native and WASM platforms
4. Update documentation for architectural changes

### Development Guidelines
- **Components**: Plain structs in `includes/components/`
- **Systems**: Free functions in `includes/systems/`
- **Entities**: Factory functions in `includes/entities/factory.hpp`
- **Levels**: TOML files in `src/assets/levels/`

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **raylib** by Ramon Santamaria - Amazing game framework
- **Box2D** by Erin Catto - Excellent 2D physics engine
- **toml11** by Toru Niina - Modern C++ TOML parser
- **Emscripten** team - WebAssembly toolchain

## 📬 Contact

- **Repository**: [the-impale-game](https://github.com/matpdev/the-impale-game)
- **Issues**: [GitHub Issues](https://github.com/matpdev/the-impale-game/issues)

---

**Made with ❤️ using raylib and Box2D**
