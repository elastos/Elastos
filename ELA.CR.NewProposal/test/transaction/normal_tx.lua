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

-- amount, fee, recipent
local amount = 0.1
local fee = 0.001
local recipient = "EbWmBQQLW35s1LmKa5P55rQcjLxmfFChxT"

-- payload
local ta = transferasset.new()

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x02, 0, ta, 0)

-- input: from, amount + fee
local charge = tx:appendenough(addr, (amount + fee) * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, output_paload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
local recipient_output = output.new(asset_id, amount * 100000000, recipient, 0, default_output)
tx:appendtxout(charge_output)
tx:appendtxout(recipient_output)
-- print(charge_output:get())
-- print(recipient_output:get())

-- sign
tx:sign(wallet)
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
