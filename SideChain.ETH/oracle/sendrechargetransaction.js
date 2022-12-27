"use strict";

const common = require("./common");

const SCErrMainchainTxDuplicate = 45013
const ErrInvalidMainchainTx = 45022

module.exports = async function (json_data, res) {
    try {
         console.log("Mainchain Transaction Received: ");
        let mctxhash = json_data["params"]["txid"];
        if (mctxhash.indexOf("0x") !== 0) mctxhash = "0x" + mctxhash;
        console.log(mctxhash);
        let txprocessed = await common.web3.eth.getStorageAt(common.blackAdr, mctxhash, common.latest)
        if (txprocessed != common.zeroHash64) {
            console.log("Mainchain Trasaction Hash already processed: " + txprocessed);
            console.log("============================================================");
            common.reterr(SCErrMainchainTxDuplicate, res);
            return;
        }
        res.json({"result": txprocessed, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        console.log("==>", err);
        common.reterr(ErrInvalidMainchainTx, res);
        return;
    }
}
