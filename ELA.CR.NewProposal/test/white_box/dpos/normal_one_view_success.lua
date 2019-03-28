--- This is a test about normal arbitrator successfully collect vote and broadcast block confirm
---
local suite = dofile("test/white_box/dpos_test_suite.lua")

return suite.run_case(function()

    suite.dpos.set_on_duty(2) -- set B on duty
    suite.dpos.dump_on_duty()

    --- initial status check
    suite.test.assert_false(suite.dpos.A.manager:is_on_duty())
    suite.test.assert_true(suite.dpos.A.manager:is_status_ready())
    suite.test.assert_false(suite.dpos.A.manager:is_status_running())

    --- generate two blocks within same height
    local b1 = block.new(suite.dpos.A.manager)
    local b2 = block.new(suite.dpos.A.manager)
    suite.test.assert_not_equal(b1:hash(), b2:hash(), "two blocss should not be equal")

    --- simulate block arrive event
    local prop = proposal.new(suite.dpos.B.manager:public_key(), b1:hash(), 0)
    suite.dpos.B.manager:sign_proposal(prop)

    --- simulate proposal arrive event
    suite.dpos.push_block(suite.dpos.A, b1)
    suite.dpos.push_block(suite.dpos.A, b2)

    suite.arbiter_proposal_confirm(suite.dpos.A, prop, b1, false)
end)
