-- Copyright (c) 2017-2020 The Elastos Foundation
-- Use of this source code is governed by an MIT
-- license that can be found in the LICENSE file.
-- 

local m = require("api")

local keystore = getWallet()
local password = getPassword()

if keystore == "" then
    keystore = "keystore.dat"
end
if password == "" then
    password = "123"
end

local wallet = client.new(keystore, password, false)

-- account
local addr = wallet:get_address()
local pubkey = wallet:get_publickey()

print("addr", addr)
print("pubkey", pubkey)

-- asset_id
local asset_id = m.get_asset_id()

-- amount, fee, recipent
--local amount = 0.2
--local fee = 0.001
--local vote_candidates = {'034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c'}
--local vote_candidate_votes = {'1.0'}

local vote_type = 0
local amount = getAmount()
local fee = getFee()
local vote_candidates = getCandidates()
local vote_candidate_votes = getCandidateVotes()

if amount == 0 then
	amount = 0.2
end

if fee == 0 then
	fee = 0.001
end

if next(vote_candidates) == nil then
    print("candidates is nil, should use --candidates or -cds to set it.")
    return
end

if next(vote_candidate_votes) == nil then
    print("candidate votes is nil, should use --candidateVotes or -cvs to set it.")
    return
end

print("amount:", amount)
print("fee:", fee)
print("vote_candidates:", vote_candidates)
print("-----------------------")
for i, v in pairs(vote_candidates) do
    print(i, v)
end
print("-----------------------")

print("vote_candidate_votes:", vote_candidate_votes)
print("-----------------------")
for i, v in pairs(vote_candidate_votes) do
    print(i, v)
end
print("-----------------------")

-- payload
local ta = transferasset.new()

-- transaction: version, tx_type, payload_version, payload, locktime
local tx = transaction.new(9, 0x02, 0, ta, 0)

-- input: from, amount + fee
local charge = tx:appendenough(addr, (amount + fee) * 100000000)
print("charge", charge)

-- votecontent: vote_type, vote_candidates, vote_candidate_votes
local vote_content = votecontent.new(vote_type, vote_candidates, vote_candidate_votes)
print("vote_content", vote_content:get())

-- outputpayload
local vote_output = voteoutput.new(0, { vote_content })
print("vote_output", vote_output:get())

local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, output_paload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
local amount_output = output.new(asset_id, amount * 100000000, addr, 1, vote_output)
-- print("txoutput", charge_output:get())
-- print("txoutput", amount_output:get())
tx:appendtxout(charge_output)
tx:appendtxout(amount_output)

print(tx:get())

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
