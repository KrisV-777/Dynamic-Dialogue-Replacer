import("core.cache.memcache")
import("core.project.config")
import("dotenv")

function main(option, default)
    if config.readonly(option:fullname()) then
        return
    end
    dotenv.load()
    local name = option:name()
    default = default or option:get("default")
    local value = os.getenv(name)
    if value then
        if type(default) == "boolean" then
            option:set_value(value:lower() == "y" or value == "1")
        else
            option:set_value(value)
        end
        return
    end
    if default ~= nil then
        if type(default) == "table" then
            t = {}
            for _, v in ipairs(default) do
                v = try { function() return vformat(v) end } or ""
                if v == "" then
                    -- bail if any member doesn't expand
                    return
                end
                table.insert(t, v)
            end
            value = path.join(table.unpack(t))
        elseif type(default) == "string" then
            value = vformat(default)
        else
            value = default
        end
        if value ~= nil then
            option:set_value(value)
        end
    end
end
