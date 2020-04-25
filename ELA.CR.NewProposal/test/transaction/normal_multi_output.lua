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

-- amounts, fee, recipent
local amounts = getAmounts()
local fee = getFee()
local recipients = getToAddr()

if amounts == 0 then
	amounts = 1.0
end

if fee == 0 then
	fee = 0.1
end

if recipients == "" then
	print("to addr is nil, should use --to to set it.")
	return
end

print("amounts:", amounts)
print("fee:", fee)
print("recipients:", recipients)

-- payload
local ta = transferasset.new()

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x02, 0, ta, 0)

-- outputpayload
local default_output = defaultoutput.new()

local amountCount = 0
local totalAmount = 0
local amountMap = {}
for a in string.gmatch(amounts, "%d+") do
	local amount = tonumber(a)
	totalAmount = totalAmount + amount
	amountMap[amountCount] = amount
	amountCount = amountCount + 1
end

local count = 0
for w in string.gmatch(recipients, "%w+") do
	count = count + 1
end

if amountCount ~= count then
	print("size of addresses and amounts not matched.")
	return
end

count = 0
for w in string.gmatch(recipients, "%w+") do
	local recipient_output = output.new(asset_id, amountMap[count] * 100000000, w, 0, default_output)
	tx:appendtxout(recipient_output)
	print("address " .. count .. ": " .. w .. ", amount: " .. amountMap[count])
	count = count + 1
end

-- input: from, amounts + fee
local charge = tx:appendenough(addr, (totalAmount + fee) * 100000000)
print(charge)

-- output: asset_id, value, recipient, output_paload_type, output_paload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
tx:appendtxout(charge_output)

-- sign
tx:sign(wallet)
print(tx:get())

-- send
local hash = tx:hash()
local res = m.send_tx(tx)

print("sending " .. hash)

if (res ~= hash) then
	print(res)
else
	print("tx send success")
end
