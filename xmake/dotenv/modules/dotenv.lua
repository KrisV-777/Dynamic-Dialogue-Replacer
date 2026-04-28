import("core.cache.memcache")
import("core.project.config")

function load()
    if memcache.get("dotenv", "loaded") then
        return
    end

    local dotenv = config.get("dotenv")
    if not dotenv then
        return
    end

    if type(dotenv) ~= "string" then
        dotenv = ".env"
    end

    dotenv = path.absolute(dotenv, "$(projectdir)")

    if not os.isfile(dotenv) then
        return
    end

    for _, line in ipairs(io.readfile(dotenv):split("\n")) do
        local k, v = line:match("^([%w_]+)%s*=%s*(.*)$")
        if k and v then
            k = k:upper()
            v = v:trim()
            if (v:startswith("'") and v:endswith("'"))
                or (v:startswith('"') and v:endswith('"')) then
                v = v:sub(2, -2)
            end
            os.setenv(k, v)
            memcache.set2("dotenv", "values", k, v)
        end
    end

    memcache.set("dotenv", "loaded", true)
end
