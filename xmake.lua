set_xmakever("2.9.5")

-- Globals
PROJECT_NAME = "DynamicDialogueReplacer"

-- Project
set_project(PROJECT_NAME)
set_version("1.2.0")
set_languages("cxx23")
set_license("gplv3")
set_warnings("allextra", "error")

-- Options
option("copy_to_papyrus")
    set_default(true)
    set_description("Copy finished build to Papyrus SKSE folder")
option_end()

-- Dependencies & Includes
-- https://github.com/xmake-io/xmake-repo/tree/dev
add_requires("yaml-cpp", "sol2", "frozen", "magic_enum", "nlohmann_json")
add_requires("luajit", { configs = { gc64 = true } })

includes("lib/commonlibsse-ng")

target("detours")
    set_kind("static")
    set_languages("c++17")
    add_headerfiles("lib/detours/src/*.h", { prefixdir = "detours" })
    add_includedirs("lib/detours/src")
    add_files("lib/detours/src/*.cpp")
    remove_files("lib/detours/src/uimports.cpp")
    set_installdir("build/detours")
target_end()

-- policies
set_policy("package.requires_lock", true)

-- rules
add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("fastest")
    set_symbols("debug")
end

-- set_config("skse_xbyak", true)
set_config("skyrim_se", true)
set_config("skyrim_ae", true)
set_config("skyrim_vr", true)

-- Target
target(PROJECT_NAME)
    -- Dependencies
    add_packages("yaml-cpp", "luajit", "sol2", "frozen", "magic_enum", "nlohmann_json")
    add_deps("detours")
    add_includedirs("lib/detours/src")

    -- CommonLibSSE
    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = PROJECT_NAME,
        author = "Scrab Joséline",
        description = "Extended implementation of Mfg Fix with new functions and animated expression transitions."
    })

    -- Source files
    set_pcxxheader("src/PCH.h")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")

    -- flags
    add_cxxflags(
        "cl::/cgthreads4",
        "cl::/diagnostics:caret",
        "cl::/external:W0",
        "cl::/fp:contract",
        "cl::/fp:except-",
        "cl::/guard:cf-",
        "cl::/Zc:enumTypes",
        "cl::/Zc:preprocessor",
        "cl::/Zc:templateScope",
        "cl::/utf-8"
    )
    -- flags (cl: warnings -> errors)
    add_cxxflags(
        "cl::/we4715", -- `function` : not all control paths return a value
        "cl::/we4189" -- `variable` : local variable is initialized but not referenced
    )
    -- flags (cl: disable warnings)
    add_cxxflags(
        "cl::/wd4068", -- unknown pragma 'clang'
        "cl::/wd4201", -- nonstandard extension used : nameless struct/union
        "cl::/wd4265" -- 'type': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
    )
    -- Conditional flags
    if is_mode("debug") then
        add_cxxflags("cl::/bigobj")
    elseif is_mode("release") then
        add_cxxflags("cl::/Zc:inline", "cl::/JMC-", "cl::/Ob3")
    end

    -- Post Build 
    after_build(function (target)
        local mod_folder = os.getenv("XSE_TES5_MODS_PATH")
        local game_folder = os.getenv("XSE_TES5_GAME_PATH")
        if game_folder then
            -- local compiler_folder = path.join(game_folder, "Papyrus Compiler/PapyrusCompiler.exe")
            -- local script_source = "dist/source/scripts"
            -- local script_output = "dist/scripts"
            -- local flags_file = path.join(game_folder, "Data/Source/Scripts/TESV_Papyrus_Flags.flg")
            -- os.execv(compiler_folder, { script_source, 
            --     "-i=" .. script_source .. ";" .. path.join(game_folder, "Data/Source/Scripts"), 
            --     "-o=" .. script_output, 
            --     "-f=" .. flags_file, 
            --     "-optimize", "-all" })
            local plugin_folder = "dist/SKSE/Plugins"
            if not os.isdir(plugin_folder) then
                os.mkdir(plugin_folder)
            end
            os.cp(target:targetfile(), plugin_folder)
            os.cp(target:symbolfile(), plugin_folder)

            if (is_mode("release")) then
                local release_folder = path.join(os.projectdir(), ".release")
                local zipfile = path.join(release_folder, target:basename() .. "-" .. target:version() .. ".7z")
                local files = path.join(os.projectdir(), "dist/*")
                if not os.isdir(release_folder) then
                    os.mkdir(release_folder)
                end
                os.exec("7z a " .. zipfile .. " " .. files)
            end
        else
            print("Warning: GamePath not defined. Skipping script compilation.")
        end
        if mod_folder and has_config("copy_to_papyrus") then
            local SkyrimPath = path.join(mod_folder, target:basename())
            os.cp("dist/*", SkyrimPath)
        else
            print("Warning: SkyrimPath not defined. Skipping post-build copy.")
        end
    end)
target_end()
