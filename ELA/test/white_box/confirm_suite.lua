local api = require("api")
local dpos_msg = require("test/white_box/dpos_msg")

local dpos = dofile("test/white_box/dpos_manager.lua")
local test = dofile("test/common/test_utils.lua")

local suite = {}

function suite.genrate_confirm_suite(prop, block)

    local confirm_suite = {}

    confirm_suite.votes = {}
    local vote_idex = 1

    confirm_suite.va = vote.new(prop:hash(), dpos.A.manager:public_key(), true)
    dpos.A.manager:sign_vote(confirm_suite.va)
    confirm_suite.votes[vote_idex] = confirm_suite.va
    vote_idex = vote_idex + 1

    confirm_suite.vb = vote.new(prop:hash(), dpos.B.manager:public_key(), true)
    dpos.B.manager:sign_vote(confirm_suite.vb)
    confirm_suite.votes[vote_idex] = confirm_suite.vb
    vote_idex = vote_idex + 1

    confirm_suite.vc = vote.new(prop:hash(), dpos.C.manager:public_key(), true)
    dpos.C.manager:sign_vote(confirm_suite.vc)
    confirm_suite.votes[vote_idex] = confirm_suite.vc
    vote_idex = vote_idex + 1

    confirm_suite.vd = vote.new(prop:hash(), dpos.D.manager:public_key(), true)
    dpos.D.manager:sign_vote(confirm_suite.vd)
    confirm_suite.votes[vote_idex] = confirm_suite.vd
    vote_idex = vote_idex + 1

    confirm_suite.ve = vote.new(prop:hash(), dpos.E.manager:public_key(), true)
    dpos.E.manager:sign_vote(confirm_suite.ve)
    confirm_suite.votes[vote_idex] = confirm_suite.ve
    vote_idex = vote_idex + 1

    confirm_suite.vf = vote.new(prop:hash(), dpos.F.manager:public_key(), true)
    dpos.F.manager:sign_vote(confirm_suite.vf)
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
        if m.name ~= dpos.current_arbitrators[i].name then
            m.network:push_vote(dpos.current_arbitrators[i].manager:public_key(),
                conf_suite.votes[i])
        end
    end

    test.assert_true(m.manager:check_last_relay(1, conf_suite.conf),
        "last relay should be the specified confirm")
end

function suite.arbiter_proposal_confirm(m, prop, block, on_duty)
    local conf_suite = suite.genrate_confirm_suite(prop, block)

    api.set_arbitrators(m.arbitrators)

    if not on_duty then
        m.network:push_proposal(prop:get_sponsor(), prop)
    end

    suite.arbiter_suite_confirm(m, conf_suite)
end

return suite
