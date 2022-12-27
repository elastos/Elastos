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

    --- initial status check
    suite.test.assert_true(suite.dpos.A.manager:is_on_duty(), "A should be on duty")
    suite.test.assert_true(suite.dpos.A.manager:is_status_ready(), "status of A should be ready")
    suite.test.assert_false(suite.dpos.A.manager:is_status_running())

    --- generate two blocks within same height
    local b1 = block.new(suite.dpos.A.manager)
    local b2 = block.new(suite.dpos.A.manager)
    suite.test.assert_not_equal(b1:hash(), b2:hash())

    --- simulate block arrive event
    suite.api.set_arbitrators(suite.dpos.A.arbitrators)

    local prop = proposal.new(suite.dpos.A.manager:public_key(), b1:hash(), 0)
    suite.dpos.A.manager:sign_proposal(prop)

    local va = vote.new(prop:hash(), suite.dpos.A.manager:public_key(), true)
    suite.dpos.push_block(suite.dpos.A, b1)
    suite.test.assert_true(suite.dpos.A.manager:is_status_running())
    suite.test.assert_true(suite.dpos.A.network:check_last_msg(
        suite.dpos_msg.accept_vote, va), "last message should be accept vote")
    suite.dpos.push_block(suite.dpos.A, b2)

    suite.arbiter_proposal_confirm(suite.dpos.A, prop, b1, false)
end)
