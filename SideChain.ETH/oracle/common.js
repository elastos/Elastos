"use strict";
const sysconst = require("./sysconst");

const Web3 = require("web3");
const web3 = new Web3(sysconst.ORACLE_SYS_CONNECTION_URL);
const ks = require("./ks_sample");
const acc = web3.eth.accounts.decrypt(ks.kstore, ks.kpass);
const sub_ctrt = require("./ctrt_sub_sample");
const main_ctrt = require("./ctrt_main_sample");
const contract = new web3.eth.Contract(sub_ctrt.abi);
contract.options.address = main_ctrt.address;
const payloadReceived = {name: null, inputs: null, signature: null};

for (const event of sub_ctrt.abi) {
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
    },
    retnum: function toNonExponential(num) {
        let m = num.toExponential().match(/\d(?:\.(\d*))?e([+-]\d+)/);
        return num.toFixed(Math.max(0, (m[1] || '').length - m[2]));
    }
}
