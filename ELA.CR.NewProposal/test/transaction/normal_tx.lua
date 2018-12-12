print("-----------start testing normal tx----------- ")

local m = require("api")

-- open keystore
wallet = client.new("keystore.dat", "123", false)

-- account
addr = wallet:getAddr()
pubkey = wallet:getPubkey()

print(addr)
print(pubkey)

-- assetID
assetID = m.getAssetID()

-- payload
ta = transferasset.new()
-- transaction
tx = transaction.new(0, 0x02, 0, ta, 0)

-- input
-- txinput = input.new("6d5dce4321d47648008bce84daea2a1e100131801ab0a48fe950e9fbab3bebfd", 1, 0xffffffff)
-- tx:appendtxin(txinput)
charge = tx:appendenough(addr, 0.02)
print(charge)

-- fee(sela)
fee = 100000

-- outputpayload
txoutputpayload = defaultoutput.new()

-- output
txoutput = output.new(assetID, charge - fee, addr, 0, txoutputpayload)
tx:appendtxout(txoutput)
-- print(txoutput:get())

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
