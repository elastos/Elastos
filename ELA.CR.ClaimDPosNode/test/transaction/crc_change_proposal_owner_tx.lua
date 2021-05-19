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
if fee == 0
    then
    fee = 0.0001
end

local target_hash = getTargetHash()
if target_hash == "" then
    print("target_hash is nil, should use --targethash to set it.")
    return
end

local owner_public_key = getOwnerPublicKey()
if owner_public_key == "" then
    print("owner_public_key is nil, should use --ownerpublickey to set it.")
    return
end

local owner_private_key = getOwnerPrivateKey()
if owner_private_key == "" then
    print("owner_private_key is nil, should use --ownerprivatekey to set it.")
    return
end

local new_owner_public_key = getNewOwnerPublicKey()
if new_owner_public_key == "" then
    print("new_owner_public_key is nil, should use --newownerpublickey to set it.")
    return
end

local new_owner_private_key = getNewOwnerPrivateKey()
if new_owner_private_key == "" then
    print("new_owner_private_key is nil, should use --newownerprivatekey to set it.")
    return
end

local proposal_type = getProposalType()
local recipient_addr = getRecipient()

print("fee:", fee)
print("proposal type:", proposal_type)
print("recipient addr:",recipient_addr)
print("target proposal hash:", target_hash)
print("proposal owner key:", owner_public_key)
print("proposal owner private key:", owner_private_key)
print("new proposal owner key:", new_owner_public_key)
print("new proposal owner private key:",new_owner_private_key)

-- crc change proposalowner payload: proposal_type, recipient_addr, target_hash, owner_public_key, owner_private_key,new_owner_public_key, new_owner_private_key, wallet
local cr_payload =crchangeproposalowner.new(proposal_type, recipient_addr, target_hash, owner_public_key, owner_private_key,new_owner_public_key, new_owner_private_key, wallet)
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
