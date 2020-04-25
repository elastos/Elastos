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


-- deposit params
--local deposit_address = "DW1jxCSjnrCrtkyvbkGcUp4aPvjacXBpAM"
--local cr_publickey = "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
--local nick_name = "ela_test11"
--local url = "ela_test.org11"
--local location = "00112211"
-- fee
--local fee = 0.001

local cr_publickey = getPublicKey()
local nick_name = getNickName()
local url = getUrl()
local location = getLocation()
local fee = getFee()
local payload_version = getPayloadVersion()

if cr_publickey == ""
	then
		print("public key is nil, should use --publickey or -pk to set it.")
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

if fee == 0
	then
		fee = 0.001
end

print("public key:", cr_publickey)
print("nick_name:", nick_name)
print("url:", url)
print("location:", location)
print("payload version:", payload_version)

-- register cr payload: publickey, nickname, url, local, wallet
local up_payload =updatecr.new(cr_publickey, nick_name, url, location,
 payload_version, wallet)
print(up_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x23, payload_version, up_payload, 0)
print("tx1")
print(tx:get())

-- input: from, amount + fee
local charge = tx:appendenough(addr, fee * 100000000)
print("charge " .. charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, outputpaload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
--local amount_output = output.new(asset_id, amount * 100000000, deposit_address, 0, default_output)
tx:appendtxout(charge_output)
--tx:appendtxout(amount_output)
-- print(charge_output:get())
-- print(amount_output:get())

-- sign
tx:sign(wallet)
print("tx2 ")
print(tx:get())

-- send
local hash = tx:hash()
print("before send_tx ")

local res = m.send_tx(tx)

print("sending " .. hash)

if (res ~= hash)
then
    print(res)
else
    print("tx send success")
end
