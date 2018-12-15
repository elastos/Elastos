local colors = require 'test/common/ansicolors'

local test = {}
test.result = 0

function test.assert_true(b, msg)
    if b then
        return 0
    else
        if msg ~= nil then
            print(colors("%{red}" .. msg))
        end
        test.result = test.result + 1
        return 1
    end
end

function test.assert_false(f)
    return test.assert_true(not f)
end

function test.assert_equal(a, b, msg)
    return test.assert_true(a == b, msg)
end

function test.assert_not_equal(a, b, msg)
    return test.assert_true(a ~= b, msg)
end

function test.assert_null(n, msg)
    return test.assert_true(n == nil, msg)
end

function test.assert_not_null(n, msg)
    return test.assert_true(n ~= nil, msg)
end

return test
