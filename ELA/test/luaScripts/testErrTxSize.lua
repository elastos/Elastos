--ErrTransactionSize
print("------------start testErrTxSize------------")
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
nonce=string.rep("77", 100000)
attr = txattribute.new(0,nonce,300)
tx = transaction.new(0x80, 0, ta, 0)
tx:appendtxin(input)
tx:appendtxout(output)
tx:appendattr(attr)
tx:sign(wallet)
tx:hash()
res=m.sendRawTx(tx)
if (res ~= "invalid transaction size")
then
	print(res)
	return
else
	print("test txSize success")
end

