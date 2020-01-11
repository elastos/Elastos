### Some useful info
```bash
DID Genesis: XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ
Token Genesis: XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4

Foundation: ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw
Mining: EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U
```

### Distribute ELA to appropriate addresses in main chain and sidechains
```bash
# Send from foundation to reserve address
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw","privateKey": "79aaa0b2df79e82e687063ff7fd03579130095ec93aa2d1861ee669aabff69c3"}],"receiver": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","amount": "16000000"}]}' localhost:8091/api/1/transfer

# Send from reserve to mainchain addr 1
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EYSLC2mgk5KTLEATK6hRYJ77Umnt1Zv72N","amount": "3000000"}]}' localhost:8091/api/1/transfer

# Send from reserve to mainchain addr 2
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD","amount": "3000000"}]}' localhost:8091/api/1/transfer

# Send from reserve to DID sidechain addr
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3","amount": "100000"}]}' localhost:8091/api/1/cross/m2d/transfer
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3"}}' http://localhost:30114

# Send from reserve to Token sidechain addr
cd $GOPATH/src/github.com/elastos/Elastos.ELA.Client
rm -f keystore.dat
./ela-cli wallet --import 109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8 -p 123
./ela-cli wallet -t create --from EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s --deposit EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW --amount 100000 --fee 0.0001; ./ela-cli wallet -t sign -p 123 --file to_be_signed.txn; ./ela-cli wallet -t send --file ready_to_send.txn
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW"}}' http://localhost:40114
```

### Register your supernodes
```bash
cd $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain
```

# Register supernode 1 using mainchain addr 1
rm -f keystore.dat
./ela-cli wallet import a24ee48f308189d46a5f050f326e76779b6508d8c8aaf51a7152b903b9f42f80 -p elastos
./ela-cli wallet b
./ela-cli script --file ../test/register_mainchain-dpos-1.lua

# Register supernode 1 using mainchain addr 1
rm -f keystore.dat
./ela-cli wallet import ff6dc625cf986eae4365f69c30035608fa47518e5ada4ad99b7cbc5df7683c30 -p elastos
./ela-cli wallet b
./ela-cli script --file ../test/register_mainchain-dpos-2.lua
```

### Caste vote to supernodes
NOTE: You gotta use owner's public key here. Also, as soon as ELA is taken out of the sender address, the votes are reset
```bash
# Give 50,000 votes to Noderators supernode using mainchain addr 1
curl -X POST -H "Content-Type: application/json" -d '{
      "sender":[
          {
              "address":"EYSLC2mgk5KTLEATK6hRYJ77Umnt1Zv72N",
              "privateKey":"a24ee48f308189d46a5f050f326e76779b6508d8c8aaf51a7152b903b9f42f80"
          }
      ],
      "memo":"Voting for Noderators",
      "receiver":[
          {
              "address":"EYSLC2mgk5KTLEATK6hRYJ77Umnt1Zv72N",
              "amount":"50000",
              "candidatePublicKeys":["03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007"]
          }
      ]
  }' localhost:8091/api/1/dpos/vote

# Give 60,000 votes to KP supernode using mainchain addr 2
curl -X POST -H "Content-Type: application/json" -d '{
      "sender":[
          {
              "address":"ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD",
              "privateKey":"ff6dc625cf986eae4365f69c30035608fa47518e5ada4ad99b7cbc5df7683c30"
          }
      ],
      "memo":"Voting for KP Supernode",
      "receiver":[
          {
              "address":"ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD",
              "amount":"60000",
              "candidatePublicKeys":["03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb"]
          }
      ]
  }' localhost:8091/api/1/dpos/vote
```

### Verify that the supernodes are in fact registered and have received some votes
```bash
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params":{"start":"0","limit":2}}' http://localhost:10014
```

### Ethereum SideChain Private net setup
- ```cd $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.ETH```
- Modify spv/spv_module.go to add private net stuff
```json
func NewService(cfg *Config, s *node.Node) (*Service, error) {
	stack = s
	var chainParams *config.Params
	var spvCfg spv.DPOSConfig
	switch strings.ToLower(cfg.ActiveNet) {
	case "testnet", "test", "t":
		chainParams = config.DefaultParams.TestNet()
		spvCfg = spv.DPOSConfig{
			Config: spv.Config{
				DataDir:     cfg.DataDir,
				ChainParams: chainParams,
			},
		}
	case "regnet", "reg", "r":
		chainParams = config.DefaultParams.RegNet()
		spvCfg = spv.DPOSConfig{
			Config: spv.Config{
				DataDir:     cfg.DataDir,
				ChainParams: chainParams,
			},
		}
	default:
		chainParams = &config.DefaultParams
		chainParams.DPoSMagic = 7630403
		chainParams.DPoSDefaultPort = 20339
		chainParams.DNSSeeds = nil
		chainParams.Magic = 7630401
		t, _ := common.Uint168FromAddress("ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw")
		chainParams.Foundation = *t
		chainParams.GenesisBlock = config.GenesisBlock(t)
		chainParams.OriginArbiters = []string{
			"02677bd3dc8ea4a9ab22f8ba5c5348fc1ce4ba5f1810e8ec8603d5bd927b630b3e",
			"0232d3172b7fc139b7605b83cd27e3c6f64fde1e71da2489764723639a6d40b5b9",
		}
		chainParams.CRCArbiters = []string{
			"0386206d1d442f5c8ddcc9ae45ab85d921b6ade3a184f43b7ccf6de02f3ca0b450",
			"0353197d11802fe0cd5409f064822b896ceaa675ea596287f1e5ce009be7684f08",
		}
		chainParams.CheckAddressHeight = 101
		chainParams.VoteStartHeight = 100
		chainParams.CRCOnlyDPOSHeight = 200
		chainParams.PublicDPOSHeight = 500
		spvCfg = spv.DPOSConfig{
			Config: spv.Config{
				DataDir:     cfg.DataDir,
				ChainParams: chainParams,
				PermanentPeers: []string{"privnet-mainchain-node:20338",
					"privnet-mainchain-crc-1:20338",
					"privnet-mainchain-crc-2:20338",
					"privnet-mainchain-dpos-1:20338",
					"privnet-mainchain-dpos-2:20338",
				},
			},
		}

	dataDir = cfg.DataDir
	initLog(cfg.DataDir)
```
- Modify core/genesis.go to add private net stuff
```json
func DefaultGenesisBlock() *Genesis {
	genesis := &Genesis{
		Config:     params.MainnetChainCon
        ...
        ...
        ...
    extra = append(extra, bytes.Repeat([]byte{0x00}, 32)...)
	/*
		address1 := hexutil.MustDecode("0xd7b0ddec94d96d4c7870deac1a2fe3347b9b4b85")
    */
    address := hexutil.MustDecode("0x961386e437294f9171040e2d56d4522c4f55187d")
	extra = append(extra, address...)
    extra = append(extra, bytes.Repeat([]byte{0x00}, 65)...)
	genesis.ExtraData = extra
	return genesis
}
```
- Modify cmd/utils/flags.go to add private net stuff
```json
	BlackContractAddr = cli.StringFlag{
		Name:  "black.contract.address",
		Usage: "configue Black Contract address",
		Value: "0x491bC043672B9286fA02FA7e0d6A3E5A0384A31A",
	}
```

- ```make geth```
- Create an account for mining
- Genesis Block Hash: b0cd29490c792dbcbe75adadee415270b9e5c8ae89dfed835440f2ac606eebfc
- Genesis address: XZyAtNipJ7fdgBRhdzCoyS7A3PDSzR7u98
```
./geth --mine --datadir elastos_eth console;
personal.newAccount("elastos-privnet");
```
- Check keystore file at elastos_eth/keystore
- Because we will only be running one eth node, we won't be using any bootnode
- Start geth:
```./geth --mine --datadir elastos_eth --ethash.dagdir elastos_ethash --rpc --rpcaddr 0.0.0.0 --rpccorsdomain '*' --rpcport 8545 --rpcapi 'personal,db,eth,net,web3,txpool,miner' --unlock 0x961386e437294f9171040e2d56d4522c4f55187d --password ./eth-accounts-pass.txt```
    
- Items needed for oracle:
deployctrt.js
```
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
acc.signTransaction(tx).then((stx) => {
    web3.eth.sendSignedTransaction(stx.rawTransaction).on("transactionHash", console.log)
        .then(console.log)
        .catch(console.log);
}).catch(console.log);
```
ks.js
```
"use strict";

module.exports = {
   kstore: {"address":"840534b46b3b3bf8c1c3e4c7d34bc86933de7814","crypto":{"cipher":"aes-128-ctr","ciphertext":"2e8ed4f40c71538a12df95fa0b5b21707be75c7dd1b57e390e505659d6a4ab72","cipherparams":{"iv":"f8b3e54a710dc7ee7faae3e7870d0cc0"},"kdf":"scrypt","kdfparams":{"dklen":32,"n":262144,"p":1,"r":8,"salt":"3affb21811ef5115de926976e9b3119f92545bcfa574ba51d9200cd4d2d8531d"},"mac":"526443e3cf1e3194afbfccc9f8f7aa8ce30b5dbb7653da513851a7d8d85407f9"},"id":"c66a6ceb-1542-429f-81db-0ca916b72fd3","version":3},
   kpass: "12345678"
}
```
- Deploy oracle:
```
cd oracle;
npm init -y;
npm install web3;
npm install express;
node crosschain_oracle.js;
```
- Deploy oracle service contract 
```node deployctrt.js```

