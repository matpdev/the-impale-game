#!/usr/bin/env fish
# Build script for the-impale-game
# Simplifies switching between native and web builds

set -l SCRIPT_DIR (dirname (status --current-filename))

function print_usage
    echo "Usage: ./build.fish [native|web] [OPTIONS]"
    echo ""
    echo "Platforms:"
    echo "  native    Build for native platform (Linux/macOS/Windows)"
    echo "  web       Build for web (WebAssembly via Emscripten)"
    echo ""
    echo "Options:"
    echo "  --run, -r       Run the game after building (native only)"
    echo "  --serve, -s     Start local server after building (web only)"
    echo "  --clean, -c     Force clean reconfigure"
    echo "  --debug, -d     Build in debug mode (default: release)"
    echo "  --help, -h      Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./build.fish native --run       # Build native and run"
    echo "  ./build.fish web --serve        # Build web and serve on port 8000"
    echo "  ./build.fish native --clean     # Clean rebuild for native"
end

# Parse arguments
set -l platform ""
set -l run_after false
set -l serve_after false
set -l force_clean false
set -l build_mode "release"

if test (count $argv) -eq 0
    print_usage
    exit 1
end

for arg in $argv
    switch $arg
        case native web
            set platform $arg
        case --run -r
            set run_after true
        case --serve -s
            set serve_after true
        case --clean -c
            set force_clean true
        case --debug -d
            set build_mode "debug"
        case --help -h
            print_usage
            exit 0
        case '*'
            echo "Error: Unknown option '$arg'"
            print_usage
            exit 1
    end
end

if test -z "$platform"
    echo "Error: Platform not specified (native or web)"
    print_usage
    exit 1
end

cd $SCRIPT_DIR

echo "🎮 Building the-impale-game for $platform ($build_mode mode)..."

# Configure and build based on platform
if test "$platform" = "native"
    echo "📦 Configuring for native build..."
    
    if test "$force_clean" = true
        xmake f -p linux -a x86_64 -m $build_mode -c
    else
        xmake f -p linux -a x86_64 -m $build_mode
    end
    
    if test $status -ne 0
        echo "❌ Configuration failed!"
        exit 1
    end
    
    echo "🔨 Building native target..."
    xmake build the-impale-game
    
    if test $status -ne 0
        echo "❌ Build failed!"
        exit 1
    end
    
    echo "✅ Native build successful!"
    echo "📍 Binary: build/linux/x86_64/$build_mode/the-impale-game"
    
    if test "$run_after" = true
        echo "🚀 Running the game..."
        xmake run the-impale-game
    else
        echo ""
        echo "To run the game: xmake run the-impale-game"
    end

else if test "$platform" = "web"
    echo "📦 Configuring for web build..."
    
    if test "$force_clean" = true
        xmake f -p wasm -a wasm32 --toolchain=emcc -m $build_mode -c
    else
        xmake f -p wasm -a wasm32 --toolchain=emcc -m $build_mode
    end
    
    if test $status -ne 0
        echo "❌ Configuration failed!"
        exit 1
    end
    
    echo "🔨 Building web target..."
    xmake build the-impale-game-web
    
    if test $status -ne 0
        echo "❌ Build failed!"
        exit 1
    end
    
    echo "✅ Web build successful!"
    echo "📍 Output: build/wasm/$build_mode/"
    
    if test "$serve_after" = true
        echo "🌐 Starting local server on http://localhost:8000 ..."
        echo "   Open: http://localhost:8000/the-impale-game-web.html"
        echo ""
        cd build/wasm/$build_mode
        python -m http.server 8000
    else
        echo ""
        echo "To serve locally:"
        echo "  cd build/wasm/$build_mode"
        echo "  python -m http.server 8000"
        echo "  Open: http://localhost:8000/the-impale-game-web.html"
    end
end
