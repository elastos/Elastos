local m = require("api")

-- client: path, password, if create
local wallet = client.new("keystore.dat", "123", false)

-- account
local addr = wallet:get_address()
local pubkey = wallet:get_publickey()
print(addr)
print(pubkey)

-- deposit params
local node_publickey = "032895050b7de1a9cf43416e6e5310f8e909249dcd9c4166159b04a343f7f141b5"

-- activate producer payload: publickey, wallet
local ap_payload = activateproducer.new(node_publickey, wallet)
print(ap_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x0d, 0, ap_payload, 0)

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
