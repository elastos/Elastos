print("-----------start testing normal tx----------- ")

local m = require("api")
wallet = client.new("keystore.dat", "123", false)
addr = wallet:getAddr()
pubkey = wallet:getPubkey()

print(addr)