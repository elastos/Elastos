--- This is a test about on duty arbitrator successfully collect vote and broadcast block confirm
---
local api = require("api")
local colors = require 'test/common/ansicolors'
local dpos_msg = require("test/white_box/dpos_msg")
local log = require("test/white_box/log_config")
local block_utils = require("test/white_box/block_utils")

local dpos = dofile("test/white_box/dpos_manager.lua")
local test = dofile("test/common/test_utils.lua")

test.file_begin()

api.clear_store()
api.init_ledger(log.level, dpos.A.arbitrators)

dpos.set_on_duty(1) -- set A on duty
dpos.dump_on_duty()

--- initial status check
test.assert_true(dpos.A.manager:is_on_duty())
test.assert_true(dpos.A.manager:is_status_ready())
test.assert_false(dpos.A.manager:is_status_running())

--- prepare related data
local b1 = block_utils.height_one()
local b2 = block_utils.height_one()
test.assert_not_equal(b1:hash(), b2:hash())

local prop = proposal.new(dpos.A.manager:public_key(), b1:hash(), 0)
local prop2 = proposal.new(dpos.A.manager:public_key(), b2:hash(), 0)
dpos.A.manager:sign_proposal(prop)
dpos.A.manager:sign_proposal(prop2)

local va = vote.new(prop:hash(), dpos.A.manager:public_key(), true)
dpos.A.manager:sign_vote(va)
local vb = vote.new(prop:hash(), dpos.B.manager:public_key(), true)
dpos.B.manager:sign_vote(vb)
local vc = vote.new(prop:hash(), dpos.C.manager:public_key(), true)
dpos.C.manager:sign_vote(vc)

local confirm = confirm.new(b1:hash())
confirm:set_proposal(prop)
confirm:append_vote(va)
confirm:append_vote(vb)
confirm:append_vote(vc)

local illegal = illegal_proposals.new()
illegal:set_proposal(prop, false)
illegal:set_proposal(prop2, true)
illegal:set_header(b1:get_header(), false)
illegal:set_header(b2:get_header(), true)

--- simulate B received the two proposals
dpos.B.network:push_block(b1, false)
dpos.B.network:push_block(b2, false)

dpos.B.network:push_proposal(dpos.A.manager:public_key(), prop)
test.assert_true(dpos.B.network:check_last_msg(dpos_msg.accept_vote, vb))
dpos.B.network:push_proposal(dpos.A.manager:public_key(), prop2)
test.assert_true(dpos.B.network:check_last_msg(dpos_msg.illegal_proposals, illegal))

dpos.B.network:push_vote(dpos.A.manager:public_key(), va)
dpos.B.network:push_vote(dpos.C.manager:public_key(), vc)
test.assert_true(dpos.B.manager:check_last_relay(1, confirm))

print(colors('%{blue}dump node relays'))
print(dpos.B.manager:dump_node_relays())
print(colors('%{blue}dump arbitrators network messages'))
print(dpos.B.network:dump_msg())

--todo check illegal must exist within next two blocks

--- simulate C received B's illegal proposal message
dpos.C.network:push_block(b1, false)
dpos.C.network:push_block(b2, false)
dpos.C.network:push_illegal_proposals(dpos.B.manager:public_key(), illegal)

--todo check illegal must exist within next two blocks

--- clean up
test.file_end()
api.close_store()

return test.result
