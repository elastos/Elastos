--ErrInvalidInput
print("------------start testErrOutput------------")

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


-- no outputs
ta = transferasset.new()
input = utxotxinput.new(txhash, 1, 0xffffffff)
output = txoutput.new(assetID, 1, addr)
tx = transaction.new(0x80, 0, ta, 0)
tx:appendtxin(input)
--tx:appendtxout(output)
tx:sign(wallet)
tx:hash()
res=m.sendRawTx(tx)
if (res ~= "invalid transaction output detected")
then
	print(res)
	return
else
	print("test output success")
end

-- coinbase output is not enough
ta2 = coinbase.new()
temp=string.rep("00", 31)
zeroHash= temp.."00"
input2 = utxotxinput.new(zeroHash, 0xffff, 0xffffffff)
output2 = txoutput.new(assetID, 1, addr)
tx2 = transaction.new(0x82, 4, ta2, 0)
tx2:appendtxin(input2)
tx2:appendtxout(output2)
tx2:sign(wallet)
hh2=tx2:hash()
res=m.sendRawTx(tx2)
if (res ~= "invalid transaction output detected")
then
	print(res)
	return
else
	print("test output success")
end

--asset ID in coinbase is invalid 
FoundationAddress = "APMkDSkoc2D6jrQTukVTFMmsFnHGNDh3cG"
ta3 = coinbase.new()
temp=string.rep("00", 31)
zeroHash= temp.."00"
input3 = utxotxinput.new(zeroHash, 0xffff, 0xffffffff)
output3 = txoutput.new(zeroHash, 1, addr)
output32 = txoutput.new(zeroHash, 1, FoundationAddress)
tx3 = transaction.new(0x82, 4, ta3, 0)
tx3:appendtxin(input3)
tx3:appendtxout(output3)
tx3:appendtxout(output32)
tx3:sign(wallet)
hh3=tx3:hash()
res=m.sendRawTx(tx3)
if (res ~= "invalid transaction output detected")
then
	print(res)
	return
else
	print("test output success")
end

--no foundation address in coinbase output
ta4 = coinbase.new()
temp=string.rep("00", 31)
zeroHash= temp.."00"
input4 = utxotxinput.new(zeroHash, 0xffff, 0xffffffff)
output4 = txoutput.new(assetID, 1, addr)
tx4 = transaction.new(0x82, 4, ta4, 0)
tx4:appendtxin(input4)
tx4:appendtxout(output4)
tx4:appendtxout(output4)
tx4:sign(wallet)
hh4=tx4:hash()
res=m.sendRawTx(tx4)
if (res ~= "invalid transaction output detected")
then
	print(res)
	return
else
	print("test output success")
end

