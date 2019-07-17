"use strict";

const Web3 = require("web3");
const web3 = new Web3("http://127.0.0.1:8545");
const ks = require("./ks");
const acc = web3.eth.accounts.decrypt(ks.kstore, ks.kpass);
const ctrt = require("./ctrt");
const contract = new web3.eth.Contract(ctrt.abi);
const cdata = require("./bytecode");
const data = contract.deploy({data: cdata.data}).encodeABI();
const tx = {data: data, gas: "2000000", gasPrice: "2000000000"};
acc.signTransaction (tx) .then ((stx) => {
    console.log("coming")
    console.log(stx.rawTransaction)
    web3.eth.sendSignedTransaction(stx.rawTransaction).then(console)
});
