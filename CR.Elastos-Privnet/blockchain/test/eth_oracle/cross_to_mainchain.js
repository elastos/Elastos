Web3 = require("web3");
web3 = new Web3("http://127.0.0.1:60011");
contract = new web3.eth.Contract([{"constant":false,"inputs":[{"name":"_addr","type":"string"},{"name":"_amount","type":"uint256"},{"name":"_fee","type":"uint256"}],"name":"receivePayload","outputs":[],"payable":true,"stateMutability":"payable","type":"function"},{"payable":true,"stateMutability":"payable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":false,"name":"_addr","type":"string"},{"indexed":false,"name":"_amount","type":"uint256"},{"indexed":false,"name":"_crosschainamount","type":"uint256"},{"indexed":true,"name":"_sender","type":"address"}],"name":"PayloadReceived","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"_sender","type":"address"},{"indexed":false,"name":"_amount","type":"uint256"},{"indexed":true,"name":"_black","type":"address"}],"name":"EtherDeposited","type":"event"}])
contract.options.address="0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A"
acc = web3.eth.accounts.decrypt({"address":"4505b967d56f84647eb3a40f7c365f7d87a88bc3","crypto":{"cipher":"aes-128-ctr","ciphertext":"2c69916bbc1ed7cdbdaf20d6ca7373b82804c3100f524524058a7a2a7189f064","cipherparams":{"iv":"7860355cb8b57b7f0f86f62d1b1de78b"},"kdf":"scrypt","kdfparams":{"dklen":32,"n":262144,"p":1,"r":8,"salt":"11c154094f40c383bfca83fdb6411761900db3848caa0b3e2dbb7127481302eb"},"mac":"21eae0e645420e7f9aa5962bc01df7c07e2efe1099f3d502255be5c69d33a911"},"id":"1f717460-1ba8-46b8-aa97-b8948300695d","version":3}, "elastos-privnet");
cdata  = contract.methods.receivePayload("ETe9JPaZh1EkRbpUaY7jYuTcEsxg89BDxY", "2000000000000000000","100000000000000").encodeABI()
tx = {data: cdata,to:contract.options.address, from:acc.address, gas: "3000000", gasPrice: "20000000000"};
tx.value="2000000000000000000"
acc.signTransaction(tx).then((res)=>{
    stx=res;
    console.log(stx.rawTransaction)
    web3.eth.sendSignedTransaction(stx.rawTransaction).then(console.log)});