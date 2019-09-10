-- Copyright (c) 2017-2019 The Elastos Foundation
-- Use of this source code is governed by an MIT
-- license that can be found in the LICENSE file.
-- 

local tx_util = {}

local keystore = getWallet()
local password = getPassword()

if keystore == "" then
    keystore = "keystore.dat"
end
if password == "" then
    password = "123"
end

local wallet = client.new(keystore, password, false)
tx_util.wallet = wallet

function tx_util.common_test()
    print('common_test invoked.')
end

return tx_util
