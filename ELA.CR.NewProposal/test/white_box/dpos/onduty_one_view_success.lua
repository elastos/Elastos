--- This is a test about on duty arbitrator successfully
---
local api = require("api")
local dpos = require("test/white_box/dpos_manager")
local dpos_msg = require("test/white_box/dpos_msg")
local block_utils = require("test/white_box/block_utils")

api.clear_store()
api.init_ledger()

--- initial status check
assert(dpos.A.manager:is_on_duty())
assert(dpos.A.manager:is_status_ready())
assert(not dpos.A.manager:is_status_running())

--- generate two blocks within same height
local b1 = block_utils.height_one()
local b2 = block_utils.height_one()
assert(b1:hash() ~= b2:hash())

--- simulate block arrive event
local prop = proposal.new(dpos.A.manager:public_key(), b1:hash(), 0)
dpos.A.manager:sign_proposal(prop)

dpos.A.network:push_block(b1, false)
dpos.A.network:check_last_msg(dpos_msg.proposal, prop)
dpos.A.network:push_block(b2, false)
dpos.A.network:check_last_msg(dpos_msg.proposal, prop) -- should be first block

--- simulate other arbitrators' approve votes
local vb = vote.new(prop:hash(), dpos.B.manager:public_key(), true)
dpos.B.manager:sign_vote(vb)
dpos.A.network:push_vote(dpos.B.manager:public_key(), vb)
dpos.A.network:check_last_msg(dpos_msg.proposal, prop) -- collecting, still be first block

--- clean up
api.close_store()
print("Test success!")
