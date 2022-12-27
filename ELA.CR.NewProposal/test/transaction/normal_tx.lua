-- Copyright (c) 2017-2020 The Elastos Foundation
-- Use of this source code is governed by an MIT
-- license that can be found in the LICENSE file.
-- 

local m = require("api")

local keystore = getWallet()
local password = getPassword()

if keystore == "" then
	keystore = "keystore.dat"
end
if password == "" then
	password = "123"
end

local wallet = client.new(keystore, password, false)

-- account
local addr = wallet:get_address()
local pubkey = wallet:get_publickey()
print(addr)
print(pubkey)

-- asset_id
local asset_id = m.get_asset_id()

-- amount, fee, recipent
local amount = getAmount()
local fee = getFee()
local recipient = getToAddr()

if amount == 0
then
	amount = 1.0
end

if fee == 0
then
	fee = 0.1
end

if recipient == ""
then
	print("to addr is nil, should use --to to set it.")
	return
end

print("amount:", amount)
print("fee:", fee)
print("recipient:", recipient)

-- payload
local ta = transferasset.new()

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x02, 0, ta, 0)

-- input: from, amount + fee
local charge = tx:appendenough(addr, (amount + fee) * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, output_paload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
local recipient_output = output.new(asset_id, amount * 100000000, recipient, 0, default_output)
tx:appendtxout(charge_output)
tx:appendtxout(recipient_output)
-- print(charge_output:get())
-- print(recipient_output:get())

-- sign
tx:sign(wallet)
print(tx:get())

-- send
local hash = tx:hash()
local res = m.send_tx(tx)

print("sending " .. hash)

if (res ~= hash)
then
	print(res)
else
	print("tx send success")
end
