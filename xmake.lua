add_rules("mode.debug", "mode.release")
add_requires("raylib", "raygui", "box2d", "toml11")

add_rules("plugin.compile_commands.autoupdate", { outputdir = ".zed", lsp = "clangd" })

target("the-impale-game")
set_kind("binary")
set_configdir("$(builddir)/$(plat)/$(arch)/$(mode)")
add_configfiles("src/assets/**", { onlycopy = true, prefixdir = "" })
add_configfiles("src/assets/levels/**", { onlycopy = true, prefixdir = "levels" })
add_files("src/**.cpp")
add_packages("raylib", "raygui", "box2d", "toml11")
-- on_run(function(target)
--     os.exec("hyprctl dispatch workspace 3")
--     os.execv(target:targetfile())
-- end)

before_build(function(target)
    print("🧠 Updating include paths before build...")

    -- Run your Lua script using xmake's built-in interpreter
    os.exec("lua src/tools/update_includes.lua")

    print("✅ includePath updated!")
end)
