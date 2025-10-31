local dkjson = require("dkjson")

local properties_file = ".vscode/c_cpp_properties.json"
local compile_file = ".vscode/compile_commands.json"

-- Helper: Read file
local function read_file(path)
    local f = io.open(path, "r")
    if not f then return nil end
    local content = f:read("*a")
    f:close()
    return content
end

-- Helper: Write file
local function write_file(path, content)
    local f = io.open(path, "w")
    if not f then return false end
    f:write(content)
    f:close()
    return true
end

-- Helper: Compare two arrays
local function arrays_equal(a, b)
    if #a ~= #b then return false end
    for i = 1, #a do
        if a[i] ~= b[i] then return false end
    end
    return true
end

-- ✅ Load compile_commands.json
local compile_content = read_file(compile_file)
if not compile_content then
    print("⚠️ compile_commands.json not found — skipping update")
    os.exit(0)
end

local compile_data, _, err = dkjson.decode(compile_content)
if err then
    print("❌ Error parsing compile_commands.json:", err)
    os.exit(1)
end

-- Extract all -isystem paths
local include_paths_set = {}
for _, entry in ipairs(compile_data) do
    if entry.arguments then
        for i, arg in ipairs(entry.arguments) do
            t = {}
            for k in string.gmatch(arg, "(/home/mistergrow/.xmake/packages/.*)") do
                t[k] = true
            end
            for p in pairs(t) do
                -- print(p)
                if p then
                    local path = p
                    if path then
                        include_paths_set[path] = true
                        include_paths_set[path .. "/**"] = true
                    end
                end
            end
        end
    end
end

-- -- Convert set to list
local new_include_paths = { "${workspaceFolder}/**" }
for p in pairs(include_paths_set) do
    print(p)
    table.insert(new_include_paths, p)
end

-- ✅ Load current c_cpp_properties.json
local props_content = read_file(properties_file)
if not props_content then
    print("❌ Failed to read c_cpp_properties.json")
    os.exit(1)
end

local props_data, _, err2 = dkjson.decode(props_content)
if err2 then
    print("❌ Error parsing c_cpp_properties.json:", err2)
    os.exit(1)
end

local configurations = props_data.configurations or {}


print("🔄 Updating includePath with new paths...")

configurations[1].includePath = new_include_paths

local updated_json = dkjson.encode(props_data, { indent = true })
if write_file(properties_file, updated_json) then
    print("✅ c_cpp_properties.json updated.")
else
    print("❌ Failed to write c_cpp_properties.json")
end
