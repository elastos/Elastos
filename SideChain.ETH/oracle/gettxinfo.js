"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        console.log("Getting Sidechain Transaction Info for Hash: ");
        let sctxhash = json_data["params"]["txid"];
        if (sctxhash.indexOf("0x") !== 0) sctxhash = "0x" + sctxhash;
        console.log(sctxhash);
        console.log("============================================================");

        let txreceipt = await common.web3.eth.getTransactionReceipt(sctxhash);
        let payload = {};
        payload["crosschainassets"] = null;
        let outputindex = 0;
        let logs = txreceipt.logs;
        if (txreceipt.status) {
            for (const log of logs) {
                if (log.address === common.contract.options.address && log.topics[0] === common.payloadReceived.signature) {
                    if (outputindex === 0) {
                        payload["crosschainassets"] = new Array();
                    }
                    let event = common.web3.eth.abi.decodeLog(common.payloadReceived.inputs, log.data, log.topics.slice(1));
                    let crosschainamount = String(common.retnum(event["_crosschainamount"] / 1e18));
                    let outputamount = String(common.retnum(event["_amount"] / 1e18));
                    payload["crosschainassets"].push({
                        "crosschainaddress": event["_addr"],
                        "crosschainamount": crosschainamount,
                        "outputamount": outputamount
                    });
                    outputindex++;
                }
            }
        }
        res.json({"error": null, "id": null, "jsonrpc": "2.0", "result": {"txid": sctxhash.slice(2), "crosschainassets": payload["crosschainassets"]}});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}
