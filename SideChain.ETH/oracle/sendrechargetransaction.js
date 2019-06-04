"use strict";

const common = require("./common");

const SCErrMainchainTxDuplicate = 45013
const ErrInvalidMainchainTx = 45022

module.exports = async function (json_data, res) {
    try {
        let address= await common.web3.eth.getCoinbase();

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

        let tx = {to: common.blackAdr, data: mctxhash, from: address};
        let gas = await common.web3.eth.estimateGas(tx);
        let gasPrice = await common.web3.eth.getGasPrice();
        gasPrice = String(BigInt(gasPrice) * BigInt(15) / BigInt(10));
        tx = {
            to: common.blackAdr,
            value: "0",
            data: mctxhash,
            from: address,
            gas: gas,
            gasPrice: gasPrice
        };
        console.log(tx)
        let sctxhash = await new Promise((resolve, reject) => {
            common.web3.eth.sendTransaction(tx).on('transactionHash', function(txhash){
                console.log("Payload sent with Sidechain txHash: " + txhash + " from: " + address);
                console.log("Mainchain txHash: " + mctxhash);
                console.log("============================================================");
                resolve(txhash.slice(2));

            }).catch((err) => {
                reject(err);
            });
        });
        res.json({"result": sctxhash, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        console.log("==>", err);
        common.reterr(ErrInvalidMainchainTx, res);
        return;
    }
}
