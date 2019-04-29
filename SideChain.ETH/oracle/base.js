const sysconst = require("./sysconst");

const Web3 = require("web3");
const web3 = new Web3(sysconst.ORACLE_SYS_CONNECTION_URL);
const ks = require("./ks_sample");
const acc = web3.eth.accounts.decrypt(ks.kstore, ks.kpass);
const ctrt = require("./ctrt_main_sample");
const contract = new web3.eth.Contract(ctrt.abi);
contract.options.address = ctrt.address;


var method = contract.methods[String("upgrade")]();
var encodeABI = method.encodeABI();
//base.acc.signTransaction({data: base.encodeABI, gas: 3000000, gasPrice: 9000000000, to: base.ctrt.address,nonce:4}).then(function(xxx) {rawtx = xxx});
//base.web3.eth.sendSignedTransaction(rawtx.rawTransaction).on("transactionHash", console.log).on("receipt",console.log).then(console.log).catch(console.log)

//signTransaction({data: encodeABI,      gas: 3000000, gasPrice: 1000000000, to: '0x302DE19296bc097d36ab9cDF8bfE4fb0b969D5dA'}).then(function(xxx) {rawtx = xxx});
//web3.txpool.content().then(function(txpool) {console.log(txpool.pending[acc.address])}


module.exports = {
    web3:web3,
    ks:ks,
    acc:acc,
    ctrt:ctrt,
    contract:contract,
    encodeABI:encodeABI
}