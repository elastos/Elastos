"use strict";

const Web3 = require("web3");
const web3 = new Web3("http://127.0.0.1:6666");
const ks = require("./ks_sample");
const acc = web3.eth.accounts.decrypt(ks.kstore, ks.kpass);
const ctrt = require("./ctrt_sample");
const contract = new web3.eth.Contract(ctrt.abi);
contract.options.address = ctrt.address;
const payloadReceived = {name: null, inputs: null, signature: null};

for (const event of ctrt.abi) {
    if (event.name === "PayloadReceived" && event.type === "event") {
        payloadReceived.name = event.name;
        payloadReceived.inputs = event.inputs;
        payloadReceived.signature = event.signature;
    }
}

module.exports = {
    web3: web3,
    acc: acc,
    contract: contract,
    payloadReceived: payloadReceived,
    reterr: function(err, res) {
        console.log("Error Encountered: ");
        console.log(err.toString());
        console.log("============================================================");
        res.json({"error": err.toString(), "id": null, "jsonrpc": "2.0", "result": null});
        return;
    }
}
