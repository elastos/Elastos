local m = require("api")

-- client: path, password, if create
local wallet = client.new("keystore.dat", "123", false)

-- account
local addr = wallet:get_address()
local pubkey = wallet:get_publickey()
print(addr)
print(pubkey)

-- asset_id
local asset_id = m.get_asset_id()

-- payload
local sp_payload = sidechainpow.new(
	"3fb6e901bcc6a4eb54e239d887a70e71b1591d13215e585c059e84d612713a57",
	"2ff9861634a07389574ae8223a626188aaf301c80d3fbb8763fd13176e0ad2e3",
	100,
	wallet
)

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x05, 0, sp_payload, 0)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, output_paload
local output1 = output.new(asset_id, 0, addr, 0, default_output)
tx:appendtxout(output1)
-- print(output1:get())

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
