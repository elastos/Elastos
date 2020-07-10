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

local cr_management_public_key = getCRManagementPublicKey()
if cr_management_public_key == "" then
    print("cr_management_public_key is nil, should use --crmanagementpublickey to set it.")
    return
end

local cr_dpos_private_key = getCRDPOSPrivateKey()
if cr_dpos_private_key == "" then
    print("cr_dpos_private_key is nil, should use --crdposprivatekey to set it.")
    return
end

local cr_committee_did = getCRCommitteeDID()
if cr_committee_did == "" then
    print("cr_committee_did is nil, should use --crcommitteedid to set it.")
    return
end

print("fee:", fee)
print("cr management public key:", cr_management_public_key)
print("cr dpos private key:", cr_dpos_private_key)
print("cr committee did:", cr_committee_did)

-- crc dpos management payload: cr_management_public_key, cr_dpos_private_key, cr_committee_did, wallet
local cr_claims_dpos_payload =crdposmanagement.new(cr_management_public_key, cr_dpos_private_key, cr_committee_did, wallet)
print(cr_claims_dpos_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x31, 0, cr_claims_dpos_payload, 0)
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
