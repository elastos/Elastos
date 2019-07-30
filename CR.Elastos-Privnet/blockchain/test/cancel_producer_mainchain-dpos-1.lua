local m = require("api")

-- client: path, password, if create
local wallet = client.new("keystore.dat", "elastos", false)

-- account
local addr = wallet:get_address()
local pubkey = wallet:get_publickey()
print(addr)
print(pubkey)

-- asset_id
local asset_id = m.get_asset_id()

-- fee
local fee = 0.001

-- deposit params
local own_publickey = "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007"

-- cancel producer payload: publickey, wallet
local cp_payload = cancelproducer.new(own_publickey, wallet)
print(cp_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x0a, 0, cp_payload, 0)

-- input: from, amount + fee
local charge = tx:appendenough(addr, fee * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, outputpaload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
tx:appendtxout(charge_output)
-- print(charge_output:get())

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
