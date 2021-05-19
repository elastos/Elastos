-- Copyright (c) 2017-2020 The Elastos Foundation
-- Use of this source code is governed by an MIT
-- license that can be found in the LICENSE file.
-- 

--- This is a test about on duty arbitrator successfully collect vote and broadcast block confirm
---
local suite = dofile("test/white_box/dpos_test_suite.lua")

return suite.run_case(function()

    suite.dpos.set_on_duty(1) -- set A on duty
    suite.dpos.dump_on_duty()

    --- prepare related data about block height 1
    local b1 = block.new(suite.dpos.A.manager)
    local b2 = block.new(suite.dpos.A.manager)

    local prop = proposal.new(suite.dpos.A.manager:public_key(), b1:hash(), 0)
    local prop2 = proposal.new(suite.dpos.A.manager:public_key(), b2:hash(), 0)
    suite.dpos.A.manager:sign_proposal(prop)
    suite.dpos.A.manager:sign_proposal(prop2)

    local conf_suite = suite.generate_confirm_suite(prop, b1)

    local illegal = illegal_proposals.new()
    illegal:set_content(prop, b1:get_header(), prop2, b2:get_header())

    --- simulate B received the two proposals
    suite.api.set_arbitrators(suite.dpos.B.arbitrators)

    suite.dpos.push_block(suite.dpos.B, b1)
    suite.dpos.push_block(suite.dpos.B, b2)

    suite.dpos.B.network:push_proposal(suite.dpos.A.manager:public_key(), prop)
    suite.test.assert_true(suite.dpos.B.network:check_last_msg(suite.dpos_msg.accept_vote, conf_suite.vb),
        "should send accept vote message")

    suite.dpos.B.network:push_proposal(suite.dpos.A.manager:public_key(), prop2)
    suite.test.assert_true(suite.dpos.B.network:check_last_msg(suite.dpos_msg.illegal_proposals, illegal),
        "should send illegal proposal to aother proposal")

    suite.arbiter_suite_confirm(suite.dpos.B, conf_suite)
end)
