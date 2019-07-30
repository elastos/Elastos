"use strict";

const common = require("./common");

module.exports = async function(res) {
    try {
        console.log("Getting Sidechain Block Number...");
        console.log("============================================================");
        let blkNum = await common.web3.eth.getBlockNumber();
        if (blkNum === 0){
            common.reterr("InternalError", res);
            return;
        }
        console.log("Sidechain Block Number: " + blkNum);
        console.log("============================================================");
        res.json({"result": blkNum, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr("InternalError", res);
        return;
    }
}
