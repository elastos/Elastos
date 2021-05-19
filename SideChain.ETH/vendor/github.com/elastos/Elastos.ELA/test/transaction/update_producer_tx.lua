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

-- fee
local fee = 0.001

-- deposit params
local own_publickey = "034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c"
local node_publickey = "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
local nick_name = "ela_test1"
local url = "ela_test1.org"
local location = "112212"
local host_address = "10.10.0.2"

-- update producer payload: publickey, nickname, url, local, host, wallet
local up_payload = updateproducer.new(own_publickey, node_publickey, nick_name, url, location, host_address, wallet)
print(up_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x0b, 0, up_payload, 0)

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
