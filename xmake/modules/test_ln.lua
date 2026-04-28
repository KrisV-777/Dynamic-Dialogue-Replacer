function main()
    import("core.project.project")
    local tmp = project.tmpfile()
    io.writefile(tmp, "test")
    local res = try {
        function()
            os.ln(tmp, tmp .. '.ln')
            os.rm(tmp .. '.ln')
            return true
        end,
        finally {
        function()
            os.rm(tmp)
        end}
    } or false
    if res then
        cprint("checking for symlink ... ${color.success}ok")
    else
        cprint("checking for symlink ... ${color.nothing}${text.nothing}")
    end
    return res
end