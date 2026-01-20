local scene_io = {}

local function is_array(t)
    if type(t) ~= "table" then
        return false
    end
    local count = 0
    for k, _ in pairs(t) do
        if type(k) ~= "number" then
            return false
        end
        count = count + 1
    end
    for i = 1, count do
        if t[i] == nil then
            return false
        end
    end
    return true
end

local function escape_string(s)
    return string.format("%q", s)
end

local function serialize_value(v, indent)
    indent = indent or ""
    local t = type(v)
    if t == "number" or t == "boolean" then
        return tostring(v)
    end
    if t == "string" then
        return escape_string(v)
    end
    if t ~= "table" then
        error("unsupported type: " .. t)
    end

    local next_indent = indent .. "  "
    local lines = { "{" }

    if is_array(v) then
        for i = 1, #v do
            lines[#lines + 1] = next_indent .. serialize_value(v[i], next_indent) .. ","
        end
    else
        local keys = {}
        for k, _ in pairs(v) do
            keys[#keys + 1] = k
        end
        table.sort(keys, function(a, b)
            if type(a) == type(b) then
                return tostring(a) < tostring(b)
            end
            return type(a) < type(b)
        end)

        for _, k in ipairs(keys) do
            local key_repr
            if type(k) == "string" and k:match("^[%a_][%w_]*$") then
                key_repr = k
            else
                key_repr = "[" .. serialize_value(k, next_indent) .. "]"
            end
            lines[#lines + 1] = next_indent .. key_repr .. " = " .. serialize_value(v[k], next_indent) .. ","
        end
    end

    lines[#lines + 1] = indent .. "}"
    return table.concat(lines, "\n")
end

function scene_io.save(path, scene)
    local ok, payload = pcall(function()
        return "return " .. serialize_value(scene) .. "\n"
    end)
    if not ok then
        return false, payload
    end

    local f, err = io.open(path, "w")
    if not f then
        return false, err
    end
    f:write(payload)
    f:close()
    return true
end

function scene_io.load(path)
    local f = io.open(path, "r")
    if not f then
        return nil, "file not found: " .. path
    end
    f:close()

    local ok, result = pcall(dofile, path)
    if not ok then
        return nil, result
    end
    if type(result) ~= "table" then
        return nil, "invalid scene file (expected table)"
    end
    return result
end

return scene_io
