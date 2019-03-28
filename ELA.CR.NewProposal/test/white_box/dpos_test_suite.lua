local api = require("api")
local colors = require 'test/common/ansicolors'
local log = require("test/white_box/log_config")
local dpos_msg = require("test/white_box/dpos_msg")

local test = dofile("test/common/test_utils.lua")
test.file_begin()

api.clear_store()
api.init_ledger(log.level)

local dpos = dofile("test/white_box/dpos_manager.lua")

local suite = {}
suite.api = api
suite.dpos = dpos
suite.test = test
suite.dpos_msg = dpos_msg

function suite.run_case(case)
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

function suite.generate_confirm_suite(prop, block)

    local confirm_suite = {}

    confirm_suite.votes = {}
    confirm_suite.block_hash = block:hash()
    local vote_idex = 1

    confirm_suite.va = vote.new(prop:hash(),
        suite.dpos.A.manager:public_key(), true)
    suite.dpos.A.manager:sign_vote(confirm_suite.va)
    confirm_suite.votes[vote_idex] = confirm_suite.va
    vote_idex = vote_idex + 1

    confirm_suite.vb = vote.new(prop:hash(), suite.dpos.B.manager:public_key(), true)
    suite.dpos.B.manager:sign_vote(confirm_suite.vb)
    confirm_suite.votes[vote_idex] = confirm_suite.vb
    vote_idex = vote_idex + 1

    confirm_suite.vc = vote.new(prop:hash(), suite.dpos.C.manager:public_key(), true)
    suite.dpos.C.manager:sign_vote(confirm_suite.vc)
    confirm_suite.votes[vote_idex] = confirm_suite.vc
    vote_idex = vote_idex + 1

    confirm_suite.vd = vote.new(prop:hash(), suite.dpos.D.manager:public_key(), true)
    suite.dpos.D.manager:sign_vote(confirm_suite.vd)
    confirm_suite.votes[vote_idex] = confirm_suite.vd
    vote_idex = vote_idex + 1

    confirm_suite.ve = vote.new(prop:hash(), suite.dpos.E.manager:public_key(), true)
    suite.dpos.E.manager:sign_vote(confirm_suite.ve)
    confirm_suite.votes[vote_idex] = confirm_suite.ve
    vote_idex = vote_idex + 1

    confirm_suite.vf = vote.new(prop:hash(), suite.dpos.F.manager:public_key(), true)
    suite.dpos.F.manager:sign_vote(confirm_suite.vf)
    confirm_suite.votes[vote_idex] = confirm_suite.vf

    confirm_suite.conf = confirm.new(block:hash())
    confirm_suite.conf:set_proposal(prop)
    confirm_suite.conf:append_vote(confirm_suite.va)
    confirm_suite.conf:append_vote(confirm_suite.vb)
    confirm_suite.conf:append_vote(confirm_suite.vc)
    confirm_suite.conf:append_vote(confirm_suite.vd)
    confirm_suite.conf:append_vote(confirm_suite.ve)

    return confirm_suite
end

function suite.arbiter_suite_confirm(m, conf_suite)
    -- we only iterate arbiters A~E, F is not needed becase it's >= 2/3 already
    for i = 1, 5 do
        if m.name ~= suite.dpos.current_arbitrators[i].name then
            m.network:push_vote(suite.dpos.current_arbitrators[i].manager:public_key(),
                conf_suite.votes[i])
        end
    end

    suite.test.assert_true(m.manager:check_confirm_in_block_pool(
        conf_suite.block_hash), "block pool should exist the specified confirm")
end

function suite.arbiter_proposal_confirm(m, prop, block, on_duty)
    local conf_suite = suite.generate_confirm_suite(prop, block)

    suite.api.set_arbitrators(m.arbitrators)

    if not on_duty then
        m.network:push_proposal(prop:get_sponsor(), prop)
    end

    suite.arbiter_suite_confirm(m, conf_suite)
end

return suite
