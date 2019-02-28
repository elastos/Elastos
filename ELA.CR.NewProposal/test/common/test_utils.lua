local colors = require 'test/common/ansicolors'

local test = {}

test.result = 0
test.file_result = 0

function test.assert_true(b, msg)
    if b then
        return 0
    else
        if msg ~= nil then
            print(colors("%{red}" .. msg))
        else
            print(colors("%{red}" .. "assertion error"))
        end
        test.file_result = test.file_result + 1
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

function test.file_begin()
    print(colors('%{blue}-----------------    Begin    -----------------'))

    test.file_result = 0
end

function test.file_end()
    if test.file_result == 0 then
        print(colors('%{green}-----------------Test success!-----------------'))
    else
        print(colors('%{red}-----------------Test failed! -----------------'))
    end

    test.result = test.result + test.file_result
end

return test
