Web3 = require("web3");
web3 = new Web3("http://127.0.0.1:20636");
contract = new web3.eth.Contract([{"constant":false,"inputs":[{"name":"_addr","type":"string"},{"name":"_amount","type":"uint256"},{"name":"_fee","type":"uint256"}],"name":"receivePayload","outputs":[],"payable":true,"stateMutability":"payable","type":"function"},{"payable":true,"stateMutability":"payable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":false,"name":"_addr","type":"string"},{"indexed":false,"name":"_amount","type":"uint256"},{"indexed":false,"name":"_crosschainamount","type":"uint256"},{"indexed":true,"name":"_sender","type":"address"}],"name":"PayloadReceived","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"_sender","type":"address"},{"indexed":false,"name":"_amount","type":"uint256"},{"indexed":true,"name":"_black","type":"address"}],"name":"EtherDeposited","type":"event"}])
contract.options.address="0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A"
acc = web3.eth.accounts.decrypt({"address":"840534b46b3b3bf8c1c3e4c7d34bc86933de7814","crypto":{"cipher":"aes-128-ctr","ciphertext":"2e8ed4f40c71538a12df95fa0b5b21707be75c7dd1b57e390e505659d6a4ab72","cipherparams":{"iv":"f8b3e54a710dc7ee7faae3e7870d0cc0"},"kdf":"scrypt","kdfparams":{"dklen":32,"n":262144,"p":1,"r":8,"salt":"3affb21811ef5115de926976e9b3119f92545bcfa574ba51d9200cd4d2d8531d"},"mac":"526443e3cf1e3194afbfccc9f8f7aa8ce30b5dbb7653da513851a7d8d85407f9"},"id":"c66a6ceb-1542-429f-81db-0ca916b72fd3","version":3}, "12345678");
cdata  = contract.methods.receivePayload("Ef2Ug9gtJCiGt7opWzz22GZuJvB9ETDJxL", "2000000000000000000","100000000000000").encodeABI()
tx = {data: cdata,to:contract.options.address, from:acc.address, gas: "3000000", gasPrice: "20000000000"};
tx.value="200000000000000000"
 acc.signTransaction(tx).then((res)=>{
     stx=res;
     console.log(stx.rawTransaction)
     web3.eth.sendSignedTransaction(stx.rawTransaction).then(console.log)});


