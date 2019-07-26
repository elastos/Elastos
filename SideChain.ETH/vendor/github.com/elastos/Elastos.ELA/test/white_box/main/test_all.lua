local api = require("api")
local colors = require 'test/common/ansicolors'

local result = 0

local function do_files(files, base_path)

    local result = 0
    for i = 1, #files do
        print("test file" .. files[i])
        local temp = dofile(files[i])

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
