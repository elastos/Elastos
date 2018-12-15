local api = require("api")
local colors = require 'test/common/ansicolors'

local result = 0

local function do_files(inputstr, sep, base_path)
    if sep == nil then
        sep = "%s"
    end

    local file_count = 1
    local test_files = {}
    for str in string.gmatch(inputstr, "([^" .. sep .. "]+)") do
        test_files[file_count] = base_path .. str
        file_count = file_count + 1
    end

    local result = 0
    for i = 1, file_count - 1 do
        local temp = dofile(test_files[i])

        if temp == nil then
            result = result + 1
        else
            result = result + temp
        end
    end

    if result ~= 0 then
        print(colors("%{red} White box tests in " .. base_path .. "failed, failed numbers :" .. result))
    end

    return result
end

local dpos_dir = "test/white_box/dpos/"
local dpos_files = api.get_dir_all_files(dpos_dir)
result = result + do_files(dpos_files, ",", dpos_dir)

assert(result == 0)
