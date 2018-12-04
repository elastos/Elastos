"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        console.log("Checking Crosschain Transactions: ");
        let txs = json_data["params"]["txs"];
        console.log(JSON.stringify(txs));
        console.log("============================================================");
        let extxs = new Array();
        for (let tx of txs) {
            if (tx.indexOf("0x") !== 0) tx = "0x" + tx;
            let txexist = await common.contract.methods.txProcessed(tx).call();
            if (txexist) {
                extxs.push(tx);
            }
        }
        res.json({"result": {"txs": extxs}, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}