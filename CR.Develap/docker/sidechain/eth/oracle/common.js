"use strict";

const Web3 = require("web3");
const web3 = new Web3("http://172.17.0.1:60112");
const ctrt = require("./ctrt");

const contract = new web3.eth.Contract(ctrt.abi);
console.log(JSON.stringify(process.env.env));
switch (process.env.env) {
    case "rinkeby":
        console.log("0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A");
        contract.options.address = "0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A";
        break;
    case "testnet":
        console.log("0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A");
        contract.options.address = "0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A";
        break;
    case "mainnet":
        console.log("0xC445f9487bF570fF508eA9Ac320b59730e81e503");
        contract.options.address = "0xC445f9487bF570fF508eA9Ac320b59730e81e503";
        break;
    default:
        console.log("config address");
        contract.options.address = ctrt.address;
}
const payloadReceived = {name: null, inputs: null, signature: null};
const blackAdr = "0x0000000000000000000000000000000000000000";
const zeroHash64 = "0x0000000000000000000000000000000000000000000000000000000000000000";
const latest = "latest";

for (const event of ctrt.abi) {
    if (event.name === "PayloadReceived" && event.type === "event") {
        payloadReceived.name = event.name;
        payloadReceived.inputs = event.inputs;
        payloadReceived.signature = event.signature;
    }
}

module.exports = {
    web3: web3,
    contract: contract,
    payloadReceived: payloadReceived,
    blackAdr: blackAdr,
    latest: latest,
    zeroHash64: zeroHash64,
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
};
