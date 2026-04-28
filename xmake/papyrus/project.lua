import("core.base.option")
import("core.base.task")
import("core.project.config")
import("core.project.project")


function main()
    local t = assert(option.get("target"), "missing required option <target>")
    config.load()
    task.run("config")

    local target = assert(project.target(t), "invalid target: %s", t)
    local papyrus = assert(target:data("papyrus"), "not a papyrus project")
    local outdir = path.absolute(option.get("outdir"), os.projectdir())
    local filename = path.basename(target:basename()) .. '.ppj'

    --- Make `p` relative to `outdir`, or absolute if too far removed
    --- @param p string
    --- @return string
    local function _rel_path(p)
        p = path.normalize(p)
        local abs = path.absolute(p, os.projectdir())
        local rel = path.relative(abs, outdir)
        if rel:startswith(path.translate("../..")) then
            return abs
        end
        return rel
    end

    local file = assert(io.open(path.join(outdir, filename), "w"), "failed to open output file")

    file:print("<?xml version='1.0'?>")
    file:print('<PapyrusProject xmlns="PapyrusProject.xsd"')
    file:print('    Flags="%s"', papyrus.flags)
    file:print('    Game="%s"', papyrus.game)
    file:print('    Output="%s"', _rel_path(target:targetdir()))
    file:print('    Optimize="%s"', papyrus.optimize and "true" or "false")
    file:print('    Anonymize="%s"', papyrus.anonymize and "true" or "false")
    file:print('    Zip="false">')
    file:print('    <Imports>')
    for _, v in ipairs(target:get("includedirs")) do
        file:print('        <Import>%s</Import>', _rel_path(v))
    end
    file:print('    </Imports>')

    -- Generating Folders instead of Scripts to avoid a papyrus-lang exception
    -- Fuzzy match on files is not ideal, but "good enough" for most use cases

    file:print('    <Folders>')
    for _, v in ipairs(target:get("files")) do
        if v:endswith("*.psc") then
            v = _rel_path(path.directory(v))
            file:print('        <Folder>%s</Folder>', v)
        end
    end
    file:print('    </Folders>')

    -- file:print('    <Scripts>')
    -- local batch = target:sourcebatches().papyrus
    -- local sourcefiles = batch and batch.sourcefiles or {}
    -- for _, v in ipairs(sourcefiles) do
    --     file:print('        <Script>%s</Script>', _rel_path(v))
    -- end
    -- file:print('    </Scripts>')

    file:print('</PapyrusProject>')

    file:close()
end
