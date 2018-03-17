--ErrInvalidInput
print("------------start testErrTxDuplicate------------")
local m = require("elaapi")
wallet = client.new("wallet_test.dat", "pwd", false)
addr = wallet:getAddr()
pubkey = wallet:getPubkey()
assetID = m.getAssetID()

m.togglemining(false)
height = m.getCurrentBlockHeight()
txhash = m.getCoinbaseHashByHeight(height)

while(true)
do
	m.discreteMining(1)
	currentHeight = m.getCurrentBlockHeight()
	print("current height:", currentHeight)
	if ((currentHeight - height) > 10)
	then
		break
	end
end


ta = transferasset.new()
input = utxotxinput.new(txhash, 1, 0xffffffff)
output = txoutput.new(assetID, 1, addr)
tx = transaction.new(0x80, 0, ta, 0)
tx:appendtxin(input)
tx:appendtxout(output)
tx:sign(wallet)
tx:hash()
m.sendRawTx(tx)
height = m.getCurrentBlockHeight()

while(true)
do
	m.discreteMining(1)
	currentHeight = m.getCurrentBlockHeight()
	print("current height:", currentHeight)
	if ((currentHeight - height) > 2)
	then
		break
	end
end

res=m.sendRawTx(tx)
if (res ~= "duplicated transaction hash detected")
then
	print(res)
	return
else
	print("test duplicate success")
end

