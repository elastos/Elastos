local api = require("api")
local colors = require 'test/common/ansicolors'
local log = require("test/white_box/log_config")
local block_utils = require("test/white_box/block_utils")
local dpos_msg = require("test/white_box/dpos_msg")

local test = dofile("test/common/test_utils.lua")
test.file_begin()

api.clear_store()
api.init_ledger(log.level)

local dpos = dofile("test/white_box/dpos_manager.lua")
local cs = require("test/white_box/confirm_suite")

local test_suit = {}
test_suit.api = api
test_suit.block_utils = block_utils
test_suit.dpos = dpos
test_suit.cs = cs
test_suit.test = test
test_suit.dpos_msg = dpos_msg

function test_suit.run_case(case)
    case()

    print(colors('%{blue}dump node relays'))
    print(dpos.A.manager:dump_node_relays())
    print(colors('%{blue}dump arbitrators network messages'))
    print(dpos.A.network:dump_msg())

    --- clean up
    test.file_end()
    api.close_store()

    return test.result
end

return test_suit
