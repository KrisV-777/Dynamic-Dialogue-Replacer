import("core.project.config")
import("lib.detect.find_program")

_REG_VALUE = "HKLM\\SOFTWARE\\WOW6432Node\\Bethesda Softworks\\Skyrim Special Edition;Installed Path"

function main(opt)
    opt = opt or {}

    opt.paths = table.wrap(opt.paths)

    table.insert(opt.paths, os.getenv("PAPYRUS_PATH") or nil)
    table.insert(opt.paths, config.get("papyrus_path") or nil)

    local function add_gamepath(p)
        if not p then
            return
        end
        p = path.join(p, "Papyrus Compiler")
        if os.isdir(p) then
            table.insert(opt.paths, p)
        end
    end

    add_gamepath(os.getenv("XSE_TES5_GAME_PATH"))
    add_gamepath(config.get("xse_tes5_game_path"))

    if is_host("windows") then
        local p = try { function() return winos.registry_query(_REG_VALUE) end }
        add_gamepath(p)
    end

    opt.check = "-?"
    local program = find_program("PapyrusCompiler", opt)

    return program
end
