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

-- assetID
local assetID = m.get_asset_id()

-- amount, fee
--local amount = 0.199
--local fee = 0.001
--local recipient = "EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR"
--local deposit_addr = "DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD"

local amount = getAmount()
local fee = getFee()
local recipient = getToAddr()
local deposit_addr = getDepositAddr()

if amount == 0
then
    amount = 4999
end

if fee == 0
then
    fee = 0.1
end

if recipient == ""
then
    recipient = "EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR"
end

if deposit_addr == ""
then
    print("deposit address is nil, should use --depositaddr or -daddr to set it.")
    return
end

print("return amount:", amount)
print("fee:", fee)
print("recipient:", recipient)
print("deposit addr:", deposit_addr)

-- return deposit payload
local rp_payload = returndepositcoin.new()
print(rp_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x0c, 0, rp_payload, 0)

-- input: from, amount + fee
local charge = tx:appendenough(deposit_addr, (amount + fee) * 100000000)
print(charge)

-- default output payload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, output_paload
local amount_output = output.new(assetID, amount * 100000000, recipient, 0, default_output)
tx:appendtxout(amount_output)
if (charge ~= 0)
then
    local charge_output = output.new(assetID, charge, deposit_addr, 0, default_output)
    tx:appendtxout(charge_output)
end
-- print(amount_output:get())

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
