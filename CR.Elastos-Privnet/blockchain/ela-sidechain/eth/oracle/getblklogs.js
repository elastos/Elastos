"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        console.log("Getting Sidechain Logs At Block Height: ");
        let blkheight = json_data["params"]["height"];
        console.log(blkheight);
        console.log("============================================================");
        let logs=null;
        if (parseInt(blkheight)>7){
            logs = await common.contract.getPastEvents(common.payloadReceived.name, {fromBlock: parseInt(blkheight)-6, toBlock: parseInt(blkheight)-6});
        }
        let result = new Array();
        let txhash = null;
        let txlog = null;
        let outputindex = 0;
        let txreceipt;
        if (logs!=null) {
            for (const log of logs) {
                if (txhash === null || txhash != log["transactionHash"]) {
                    txhash = log["transactionHash"];
                    txlog = {"txid": txhash.slice(2)};
                    result.push(txlog);
                    txreceipt = await common.web3.eth.getTransactionReceipt(txhash)
                    console.log(txreceipt);
                    if (txreceipt.status) {
                        txlog["crosschainassets"] = new Array();
                    }
                }

                if (txreceipt.status) {
                    let crosschainamount = String(common.retnum(log["returnValues"]["_crosschainamount"] / 1e18));
                    let outputamount = String(log["returnValues"]["_amount"] / 1e18);
                    txlog["crosschainassets"].push({
                        "crosschainaddress": log["returnValues"]["_addr"],
                        "crosschainamount": crosschainamount,
                        "outputamount": outputamount
                    });
                    outputindex++;
                }
            }
        }
        res.json({"result": result, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}
