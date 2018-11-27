print("-----------start testing vote tx----------- ")

local c = dofile("test/test_script/common.lua")
local m = require("api")

-- open keystore
wallet = client.new("keystore.dat", "123", false)

-- account
addr = wallet:getAddr()
pubkey = wallet:getPubkey()

print("addr", addr)
print("pubkey", pubkey)

-- assetID
assetID = m.getAssetID()

-- payload
ta = transferasset.new()
-- transaction
tx = transaction.new(9, 0x02, 0, ta, 0)

-- input
-- txinput = input.new("6d5dce4321d47648008bce84daea2a1e100131801ab0a48fe950e9fbab3bebfd", 1, 0xffffffff)
-- tx:appendtxin(txinput)
charge = tx:appendenough(addr, 0.02)
print("charge", charge)

-- fee(sela)
fee = 100000

-- votecontent
txvotecontent = votecontent.new(0, {'8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta', 'Eds4UjJCqGfAYknGKu5GJpY6Rd4TRHrjiS', 'EadJqRKc7YeXNRUettRbg6MNdpJsJUhxoE'})
print("txvotecontent", txvotecontent:get())

-- outputpayload
txoutputpayload = voteoutput.new(0, {txvotecontent})
print("txoutputpayload", txoutputpayload:get())

-- output
txoutput = output.new(assetID, charge - fee, addr, 1, txoutputpayload)
print("txoutput", txoutput:get())

tx:appendtxout(txoutput)

print(tx:get())

-- sign
tx:sign(wallet)
print(tx:get())

-- send
hash = tx:hash()
res = m.sendTx(tx)

print("sending " .. hash)

if (res ~= hash)
then
	print(res)
else
	print("tx send success")
end
