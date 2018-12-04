"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        console.log("Mainchain Transaction Received: ");
        let mctxhash = json_data["params"]["info"]["payload"]["mainchaintxhash"];
        if (mctxhash.indexOf("0x") !== 0) mctxhash = "0x" + mctxhash;
        console.log(mctxhash);
        console.log("============================================================");

        let txprocessed = await common.contract.methods.txProcessed(mctxhash).call();
        if (txprocessed) {
            console.log("Mainchain Trasaction Hash already processed: " + mctxhash);
            console.log("============================================================");
            common.reterr("ErrMainchainTxDuplicate", res);
            return;
        }

        let data = common.contract.methods.sendPayload(mctxhash).encodeABI();
        let tx = {to: common.contract.options.address, data: data, from: common.acc.address};
        let gas = await common.web3.eth.estimateGas(tx);
        let gasPrice = await common.web3.eth.getGasPrice();
        gas = String(BigInt(gas) * BigInt(12) / BigInt(10));
        gasPrice = String(BigInt(gasPrice) * BigInt(12) / BigInt(10));

        tx = {to: common.contract.options.address, data: data, value: "0", gas: gas, gasPrice: gasPrice};
        let stx = await common.acc.signTransaction(tx);
        let sctxhash = await new Promise((resolve, reject) => {
            common.web3.eth.sendSignedTransaction(stx.rawTransaction).on("transactionHash", (txhash) => {
                console.log("Payload sent with Sidechain txHash: " + txhash + " from: " + common.acc.address);
                console.log("Mainchain txHash: " + mctxhash);
                console.log("============================================================");
                resolve(txhash);
            }).catch((err) => {
                reject(err);
            });
        });
        res.json({"result":[sctxhash], "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr("InvalidParams", res);
        return;
    }
}
