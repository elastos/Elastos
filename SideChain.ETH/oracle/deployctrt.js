"use strict";

const Web3 = require("web3");
const web3 = new Web3("http://127.0.0.1:8545");
const address= "0x840534b46b3b3bf8c1c3e4c7d34bc86933de7814";
const ctrt = require("./ctrt");
const contract = new web3.eth.Contract(ctrt.abi);
const cdata = require("./bytecode");
const data = contract.deploy({data: cdata.data}).encodeABI();
const tx = {data: data, gas: "8000000", gasPrice: "200000",from :address};
web3.eth.sendTransaction(tx).on("transactionHash", console.log).then(console.log) .catch(console.log);


