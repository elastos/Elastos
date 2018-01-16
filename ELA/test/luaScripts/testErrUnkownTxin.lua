--ErrInvalidInput
print("------------start testErrUnknownTxin------------")
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


-- GetReference failed, refIdx out of range.
ta = transferasset.new()
input = utxotxinput.new(txhash, 2, 0xffffffff)
output = txoutput.new(assetID, 1, addr)
tx = transaction.new(0x80, 0, ta, 0)
tx:appendtxin(input)
tx:appendtxout(output)
tx:sign(wallet)
tx:hash()
res=m.sendRawTx(tx)
if (res ~= "transaction balance unmatched")
then
	print(res)
	return
else
	print("test unkowntxin success")
end

