add_rules("mode.debug", "mode.release")

-- Platform-specific package requirements
if is_plat("wasm") then
    add_requires("raylib", {configs = {platform = "Web"}})
    add_requires("box2d", "toml11")
    -- raygui not supported on WASM yet
else
    add_requires("raylib", "raygui", "box2d", "toml11")
end

add_rules("plugin.compile_commands.autoupdate", { outputdir = ".zed", lsp = "clangd" })

-- Native build target (Linux/Windows/macOS)
target("the-impale-game")
    set_kind("binary")
    set_configdir("$(builddir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("src/assets/**", { onlycopy = true, prefixdir = "" })
    add_configfiles("src/assets/levels/**", { onlycopy = true, prefixdir = "levels" })
    add_configfiles("src/assets/ads/**", { onlycopy = true, prefixdir = "ads" })
    add_files("src/**.cpp")
    add_packages("raylib", "raygui", "box2d", "toml11")
    -- on_run(function(target)
    --     os.exec("hyprctl dispatch workspace 3")
    --     os.execv(target:targetfile())
    -- end)

    -- before_build(function(target)
    --     print("🧠 Updating include paths before build...")
    --     -- Run the update_includes script (optional - don't fail build if missing)
    --     local script_path = path.join(os.scriptdir(), "src/tools/update_includes.lua")
    --     if os.isfile(script_path) then
    --         os.execv("lua", {script_path})
    --     else
    --         print("⚠️  Warning: update_includes.lua not found, skipping...")
    --     end
    -- end)
target_end()

-- Web build target (Emscripten/WASM)
target("the-impale-game-web")
    set_kind("binary")
    set_plat("wasm")
    set_arch("wasm32")
    
    -- Use Emscripten toolchain
    set_toolchains("emcc")
    
    -- Output configuration
    set_configdir("$(builddir)/wasm/$(mode)")
    set_targetdir("$(builddir)/wasm/$(mode)")
    set_extension(".html")
    
    -- Source files and assets
    add_files("src/**.cpp")
    add_configfiles("src/assets/**", { onlycopy = true, prefixdir = "assets" })
    add_configfiles("src/assets/levels/**", { onlycopy = true, prefixdir = "assets/levels" })
    add_configfiles("src/assets/ads/**", { onlycopy = true, prefixdir = "assets/ads" })
    
    -- Add WASM packages (no raygui - unsupported)
    add_packages("raylib", "box2d", "toml11")
    
    -- Emscripten-specific settings
    add_ldflags(
        "-s USE_GLFW=3",
        "-s ASYNCIFY",
        "-s INITIAL_MEMORY=67108864",  -- 64MB
        "-s ALLOW_MEMORY_GROWTH=1",
        "-s FORCE_FILESYSTEM=1",
        "--preload-file $(builddir)/wasm/$(mode)/assets@assets"
    )
    
    -- Optimization flags for web
    if is_mode("release") then
        add_ldflags("-O3")
    end
target_end()
