add_moduledirs("modules")

option("papyrus_path")
    set_description("Path to Papyrus Compiler directory")
    set_category("papyrus")
    on_check(function(option)
        import("core.project.config")
        import("lib.detect.find_tool")
        if config.readonly(option:fullname()) then
            return
        end
        local papyrus = find_tool("papyrus")
        if papyrus then
            option:set_value(papyrus.program)
        end
    end)
    after_check(function(option)
        import("lib.detect.find_tool")
        if not option:value() then
            return
        end
        if not find_tool("papyrus") then
            cprint("${color.warning}PapyrusCompiler not found in papyrus_path")
        end
    end)
option_end()

task("papyrus.anonymize")
    on_run("papyrus.anonymize")
    set_menu {
        usage = "xmake papyrus.anonymize <file>",
        description = "Anonymize the pex header.",
        options =  {
            {nil, "file", "v", nil, "The file to anonymize." },
        }
    }
task_end()

task("papyrus.project")
    on_run("project")
    set_menu {
        usage = "xmake papyrus.project <target>",
        description = "Generate a papyrus project file.",
        options = {
            {'o', "outdir", "kv", ".", "Set the output directory." },
            {nil, "target", "v", nil, "The target to generate."},
        }
    }

rule("papyrus")
    set_extensions(".psc")
    on_config("papyrus.on_config")
    on_build_file("papyrus.on_build_file")
rule_end()
