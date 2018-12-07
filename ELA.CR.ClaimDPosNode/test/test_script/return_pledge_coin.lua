print("-----------start testing return pledge coin tx----------- ")

local m = require("api")

-- open keystore
wallet = client.new("keystore_pledge.dat", "123", false)

-- account
addr = wallet:getAddr()
pubkey = wallet:getPubkey()

print(addr)
print(pubkey)

-- assetID
assetID = m.getAssetID()

-- payload
payloadRP = returnpledgecoin.new()
print(payloadRP:get())
-- transaction
tx = transaction.new(9, 0x0c, 0, payloadRP, 0)

-- input
charge = tx:appendenough(addr, 0.02)
print(charge)

-- fee(sela)
fee = 100000

-- outputpayload
txoutputpayload = defaultoutput.new()

-- output
recipient = "EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR"
txoutput = output.new(assetID, charge - fee, recipient, 0, txoutputpayload)
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
