--Nomal block
print("------------start testNormalBlock------------")
local m = require("elaapi")
wallet = client.new("wallet_test.dat", "pwd", false)
addr = wallet:getAddr()
pubkey = wallet:getPubkey()
assetID = m.getAssetID()

m.togglemining(false)
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

-- add coinbase
FoundationAddress = "APMkDSkoc2D6jrQTukVTFMmsFnHGNDh3cG"
ta0 = coinbase.new()
temp=string.rep("00", 31)
zeroHash= temp.."00"
input0 = utxotxinput.new(zeroHash, 0xffff, 0)
output0 = txoutput.new(assetID, 3, addr)
output01 = txoutput.new(assetID, 7, FoundationAddress)
nonce="00123456"
attr = txattribute.new(0,nonce,10)
tx0 = transaction.new(0x82, 4, ta0, 10)
tx0:appendtxin(input0)
tx0:appendtxout(output0)
tx0:appendtxout(output01)
tx0:appendattr(attr)
tx0:sign(wallet)
tx0:hash()

prevHeight = m.getCurrentBlockHeight()
prevHash = m.getCurrentBlockHash()
prevTimeStamp = m.getCurrentTimeStamp()
bits = m.getLatestBits()
header = blockdata.new(0, prevHash, prevHash,prevTimeStamp+1 , bits,prevHeight+1, 0)
block = block.new(header)
block:appendtx(tx0)
block:updataRoot()
block:mining()
block:hash()
res = m.submitBlock(block)
if (res ~= true) 
then
	print("submit block err")
else
	print("submit block success")
end
