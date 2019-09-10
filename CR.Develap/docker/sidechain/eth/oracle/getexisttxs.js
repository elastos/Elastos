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
            let txprocessed = await common.web3.eth.getStorageAt(common.blackAdr, tx, common.latest)
            if (txprocessed != common.zeroHash64) {
                extxs.push(tx);
                continue;
            }
        }
        res.json({"result": extxs, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}