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
print("wallet addr:", addr)
print("wallet public key:", pubkey)

-- asset_id
local asset_id = m.get_asset_id()

local fee = getFee()
local cr_pubkey = getPublicKey()
local proposal_type = getProposalType()

local previous_hash = getPreviousHash()
local change_proposal_owner_key = getChangeProposalOwnerKey()

if fee == 0
    then
    fee = 0.0001
end

if cr_pubkey == "" then
    cr_pubkey = pubkey
end

if previous_hash == "" then
    print("previous_hash is nil, should use --previoushash to set it.")
    return
end

if change_proposal_owner_key == "" then
    print("change_proposal_owner_key is nil, should use --newownerpublickey to set it.")
    return
end

print("fee:", fee)
print("public key:", cr_pubkey)
print("proposal type:", proposal_type)
print("previous proposal hash:", previous_hash)
print("new proposal owner key:", change_proposal_owner_key)

-- crc change proposalowner payload: crPublickey, proposalType, change_proposal_owner_key, previous_hash, wallet
local cr_payload =crchangeproposalowner.new(cr_pubkey, proposal_type, change_proposal_owner_key, previous_hash, wallet)
print(cr_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x25, 0, cr_payload, 0)
print(tx:get())

-- input: from, fee
local charge = tx:appendenough(addr, fee * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, outputpaload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
tx:appendtxout(charge_output)

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
