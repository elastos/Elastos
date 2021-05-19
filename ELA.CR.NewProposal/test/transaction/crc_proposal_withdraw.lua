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
local CRExpensesAddress = getCRExpensesAddress()
-- asset_id
local asset_id = m.get_asset_id()
local proposal_hash = getProposalHash()
local amount = getAmount()
local fee = getFee()
local recipient = getToAddr()
local payloadversion = getPayloadVersion()

print("CRExpensesAddress " .. CRExpensesAddress)
print("amount " .. amount)
print("fee " .. fee)
print("addr " .. addr)
print("recipient " .. recipient)
print("payloadversion" .. payloadversion)

local crcproposalwithdraw_payload =crcproposalwithdraw.new(proposal_hash, wallet, payloadversion,recipient,amount * 100000000, fee * 100000000)
-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x29, payloadversion, crcproposalwithdraw_payload, 0)
print(tx:get())

local using_addr = CRExpensesAddress
if payloadversion == 1 then
    using_addr = addr
end
local charge = tx:appendenough(using_addr, amount * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()
local amount_output = output.new(asset_id, amount * 100000000-fee*
        100000000, recipient, 0, default_output)
tx:appendtxout(amount_output)


local charge_output = output.new(asset_id, charge, using_addr, 0, default_output)
tx:appendtxout(charge_output)
print(tx:get())

if payloadversion == 1 then
    -- sign
    tx:sign(wallet)
    print(tx:get())
end

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
