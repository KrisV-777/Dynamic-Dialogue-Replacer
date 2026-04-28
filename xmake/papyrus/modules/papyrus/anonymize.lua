import("core.base.bytes")
import("core.base.option")

function main(file)
    file = file or option.get("file")
    assert(file, "Missing required option <file>")
    local f = assert(io.open(file, "rb"), "Failed to open file: " .. file)
    local buf = bytes(f:read("*a")):clone()
    f:close()
    local magic = buf:u32le(1)

    local little
    if magic == 0xFA57C0DE then
        little = true
    elseif magic == 0xDEC057FA then
        little = false
    else
        print("Invalid magic")
        return
    end

    ---@param offset integer
    ---@return string
    local function read_str(offset)
        local size = little and buf:u16le(offset) or buf:u16be(offset)
        offset = offset + 2
        local str = buf:str(offset, offset + size - 1):gsub("\0", "")
        return str
    end

    ---@param offset integer
    ---@param str string
    local function write_str(offset, str)
        local size = little and buf:u16le(offset) or buf:u16be(offset)
        local n = str:len()
        if n >= size then
            n = size
        end
        buf:copy2(offset + 2, str, 1, n)
        if n < size then
            buf:copy2(offset + 2 + n, string.rep("\0", size - n))
        end
    end

    ---@param offset integer
    ---@return integer
    local function seek_str(offset)
        local size = little and buf:u16le(offset) or buf:u16be(offset)
        return offset + size + 2
    end

    local do_write = false

    local offset = 17
    local script_path = read_str(offset)
    local script_name = path.filename(script_path)
    if script_path ~= script_name then
        write_str(offset, script_name)
        do_write = true
    end
    offset = seek_str(offset)

    local user_name = read_str(offset)
    local mask = string.rep("x", user_name:len())
    if user_name ~= mask then
        write_str(offset, mask)
        do_write = true
    end
    offset = seek_str(offset)

    local computer_name = read_str(offset)
    local mask = string.rep("X", computer_name:len())
    if computer_name ~= mask then
        write_str(offset, mask)
        do_write = true
    end

    if do_write then
        -- Ugly rewrite of whole file because xmake doesnt support "r+b" mode
        local f = assert(io.open(file, "w+b"))
        f:write(buf)
        f:flush()
        f:close()
    end
end
