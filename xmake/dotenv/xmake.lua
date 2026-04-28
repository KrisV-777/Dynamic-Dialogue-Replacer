add_moduledirs("modules")

option("dotenv")
    set_default(true)
    set_category("dotenv")
    set_description("Load environment variables from a .env file.")
    after_check(function(option)
        import("dotenv")
        import("core.project.config")

        if option:enabled() then
            return dotenv.load()
        end
    end)
option_end()
