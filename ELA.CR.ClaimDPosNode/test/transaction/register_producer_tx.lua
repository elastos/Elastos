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

-- amount, fee
--local amount = 5000
--local fee = 0.001

-- deposit params
--local deposit_address = "DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD"
--local own_publickey = "034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c"
--local node_publickey = "029628ed890a25295a91817669dd9e35b4d792d51503c7eb4190b01a26e1a48fba"
--local nick_name = "ela_test"
--local url = "ela_test.org"
--local location = "112211"
--local host_address = "127.0.0.1"

local amount = getDepositAmount()
local fee = getFee()
local deposit_address = getDepositAddr()
local own_publickey = getOwnerPublicKey()
local node_publickey = getNodePublicKey()
local nick_name = getNickName()
local url = getUrl()
local location = getLocation()
local host_address = getHostAddr()

if amount == 0
then
    amount = 5000
end

if fee == 0
then
    fee = 0.001
end

if deposit_address == ""
then
    print("deposit addr is nil, should use --depositaddr or -daddr to set it.")
    return
end

if own_publickey == ""
then
    print("owner public key is nil, should use --ownerpublickey or -opk to set it.")
    return
end

if node_publickey == ""
then
    print("node public key is nil, should use --nodepublickey or -npk to set it.")
    return
end

if nick_name == ""
then
    nick_name = "nickname_test"
end

if url == ""
then
    url = "url_test"
end

if location == ""
then
    location = 123
end

print("deposit amount:", amount)
print("fee:", fee)
print("deposit addr:", deposit_address)
print("owner public key:", own_publickey)
print("node public key:", node_publickey)
print("nick name:", nick_name)
print("url:", url)
print("location:", location)


-- register producer payload: publickey, nickname, url, local, host, wallet
local rp_payload = registerproducer.new(own_publickey, node_publickey, nick_name, url, location, host_address, wallet)
print(rp_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x09, 0, rp_payload, 0)
print(tx:get())

-- input: from, amount + fee
local charge = tx:appendenough(addr, (amount + fee) * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, outputpaload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
local amount_output = output.new(asset_id, amount * 100000000, deposit_address, 0, default_output)
tx:appendtxout(charge_output)
tx:appendtxout(amount_output)
-- print(charge_output:get())
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
