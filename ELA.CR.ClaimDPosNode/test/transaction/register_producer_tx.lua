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

-- amount, fee
local amount = 5000
local fee = 0.001

-- deposit params
local deposit_address = "DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD"
local own_publickey = "034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c"
local node_publickey = "029628ed890a25295a91817669dd9e35b4d792d51503c7eb4190b01a26e1a48fba"
local nick_name = "ela_test"
local url = "ela_test.org"
local location = "112211"
local host_address = "127.0.0.1"

-- register producer payload: publickey, nickname, url, local, host, wallet
local rp_payload = registerproducer.new(own_publickey, node_publickey, nick_name, url, location, host_address, wallet)
print(rp_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x09, 0, rp_payload, 0)
print(tx:get())

-- input: from, amount + fee
local charge = tx:appendenough(addr, (amount + fee) * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, outputpaload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
local amount_output = output.new(asset_id, amount * 100000000, deposit_address, 0, default_output)
tx:appendtxout(charge_output)
tx:appendtxout(amount_output)
-- print(charge_output:get())
-- print(amount_output:get())

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
