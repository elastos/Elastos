"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        console.log("Getting Sidechain Logs At Block Height: ");
        let blkheight = json_data["params"]["height"];
        console.log(blkheight);
        console.log("============================================================");
        let logs = await common.contract.getPastEvents(common.payloadReceived.name, {fromBlock: blkheight, toBlock: blkheight});
        let result = new Array();
        let txhash = null;
        let txlog = null;
        let outputindex = 0;
        for (const log of logs) {
            if (txhash === null || txhash != log["transactionHash"]) {
                txhash = log["transactionHash"];
                txlog = {"txid": txhash};
                result.push(txlog);
                txlog["crosschainassets"] = new Array();
            }
            txlog["crosschainassets"].push({
                "crosschainaddress": log["returnValues"]["_addr"],
                "crosschainamount": String(BigInt(log["returnValues"]["_amount"]) / BigInt("10000000000"))
            });
            outputindex++;
        }
        res.json({"result": result, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}
