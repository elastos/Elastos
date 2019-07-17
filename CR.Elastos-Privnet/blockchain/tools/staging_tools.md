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
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr","amount": "100000"}]}' localhost:8091/api/1/transfer

# Send from reserve to mainchain addr 2
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY","amount": "100000"}]}' localhost:8091/api/1/transfer

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
              "address":"EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr",
              "privateKey":"a24ee48f308189d46a5f050f326e76779b6508d8c8aaf51a7152b903b9f42f80"
          }
      ],
      "memo":"Voting for Noderators",
      "receiver":[
          {
              "address":"EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr",
              "amount":"50000",
              "candidatePublicKeys":["03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007"]
          }
      ]
  }' localhost:8091/api/1/dpos/vote

# Give 60,000 votes to KP supernode using mainchain addr 2
curl -X POST -H "Content-Type: application/json" -d '{
      "sender":[
          {
              "address":"EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY",
              "privateKey":"ff6dc625cf986eae4365f69c30035608fa47518e5ada4ad99b7cbc5df7683c30"
          }
      ],
      "memo":"Voting for KP Supernode",
      "receiver":[
          {
              "address":"EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY",
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
		//chainParams = config.DefaultParams.TestNet()
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
				DataDir:        cfg.DataDir,
				ChainParams:    chainParams,
				PermanentPeers: []string{"127.0.0.1:10016", "127.0.0.1:10116", "127.0.0.1:10216", "127.0.0.1:10516", "127.0.0.1:10616"},
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
		spvCfg = spv.DPOSConfig{
			Config: spv.Config{
				DataDir:     cfg.DataDir,
				ChainParams: chainParams,
			},
		}
	}

	dataDir = cfg.DataDir
	initLog(cfg.DataDir)
```
- Modify params/bootnodes.go to add private net stuff
```json
var TestnetBootnodes = []string{
	"enode://a3dd9132949575c702fe5ed19c481759507330c6e536bdb5cb1f3c3dab6c8f0df2781bbea0985411053b0db54aec6d87dbd8def4e29b4e402d63118d1cd3bfd4@127.0.0.1:30301",
	//"enode://5e1d6f9f74e33b2d1e2fda87efaf60a788b338c08eefd3a435e9c7de98645bc041421c27d9ed3927c7b5195febd691aff30de881842749f3030089df0e135232@3.208.184.54:30301",
	//"enode://30dc2b7986e2ec5902498ec26fad6fcecece617aa1652f227f684ede6a0939bb7a205ada1c91420d30b427c86bbdcc31fdfd6d955dd8f5854370f583025a0708@3.209.35.13:30301",
	//"enode://b0357d45e9070c1660f63f077e0e3b0054a18d93785589d498586b6e0b7ec7c5b39ef608e82e7280ca95019db7c36455275d98a3e8684916ba8f3a7aab4ad38b@3.210.227.193:30301",
}
```
- Modify core/genesis.go to add private net stuff
```json
func DefaultTestnetGenesisBlock() *Genesis {
	genesis := &Genesis{
		Config:     params.TestnetChainConfig,
		Timestamp:  0x5bda9da7,
		ExtraData:  hexutil.MustDecode("0x0000000000000000000000000000000000000000000000000000000000000000ab19cd830963611036c8f23b3aa1e143632342e70000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		GasLimit:   0x2068F7700,
		Difficulty: big.NewInt(1),
		Alloc:      nil,
	}
    return genesis
```
- ```go install ./...```
- ```cp $GOPATH/bin/abigen  /home/kpachhai/dev/bin/bootnode  $GOPATH/bin/clef  $GOPATH/bin/ethkey  $GOPATH/bin/evm  /home/kpachhai/dev/bin/examples  /home/kpachhai/dev/bin/faucet  $GOPATH/bin/geth  /home/kpachhai/dev/bin/mimegen  /home/kpachhai/dev/bin/p2psim  /home/kpachhai/dev/bin/puppeth  /home/kpachhai/dev/bin/rlpdump  /home/kpachhai/dev/bin/simulations  /home/kpachhai/dev/bin/swarm  /home/kpachhai/dev/bin/swarm-smoke  /home/kpachhai/dev/bin/wnode build/bin/```
- Create an account - create two accounts - one for mining and two for oracle service(have 2 eth sidechains running so need 2 oracle services)
- Genesis Block Hash: 0xd81e19b9e917a297ef45babb6dc7c16705bb858fe14c67b25847768149f6396c
- Genesis address: XEHPFZ9DUg7MaquN8vLKzwi1fSU9nLmFto
```
./geth --testnet --datadir elastos_eth console;
personal.newAccount("elastos-privnet");
```
- Check keystore file at elastos_eth/keystore
- Generate key file for bootnode:
```./bootnode -genkey boot_test```
- Start bootnode:
```./bootnode -nodekey boot_test -addr ":30301" -nat "ip:127.0.0.1"```
- Start geth:
```./geth --testnet --datadir elastos_eth --ethash.dagdir elastos_ethash --rpc --rpcaddr 0.0.0.0 --rpccorsdomain '*' --rpcport 8545 --rpcapi 'personal,db,eth,net,web3,txpool,miner' --unlock 0x4505b967d56f84647eb3a40f7c365f7d87a88bc3,0x268b7f52010cbbca2d910b5e67260fc119afa5c9 --password ./eth-accounts-pass.txt```
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
   kstore: {"address":"82f9dc060f38e778cf7b719d3057e814318b7f55","crypto":{"cipher":"aes-128-ctr","ciphertext":"0da9f88336c0e5a587b5178923f6618b36719b76e884b6c61769651029cd3969","cipherparams":{"iv":"600f19c3343bbc996e6884c3d4eeee22"},"kdf":"scrypt","kdfparams":{"dklen":32,"n":262144,"p":1,"r":8,"salt":"378427135244e7f532f99cace05acfe2fc1d6c0bebe06e5267acd9b0339f6066"},"mac":"188bb09bc0b342391203ae321da4ee88e5f91d1748624e09babccbef58560388"},"id":"d9624552-5d99-4953-ae80-55061766cb91","version":3},
   kpass: ""
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
- Transfer some ELA from main chain to Eth Address
1. Change directory
  ```
  cd $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain
  ```

2. Configure ela-cli config file

    Create a file called "cli-config.json" and put the following content in that file:

    ```
    {
      "Host": "127.0.0.1:10014",
      "DepositAddress":"XK6EsthrSvzkhHoeHyyodhWkmjcENtVBXy"
    }
3. Create a new wallet using ela-cli-crosschain client for testing purposes

    ```
    ./ela-cli-crosschain wallet --create -p elastos
    ```

    Save ELA address, Public key and Private key to a variable so it can be used later
    ```bash
    ELAADDRESS=$(./ela-cli-crosschain wallet -a -p elastos | tail -2 | head -1 | cut -d' ' -f1)
    PUBLICKEY=$(./ela-cli-crosschain wallet -a -p elastos | tail -2 | head -1 | cut -d' ' -f2)
    PRIVATEKEY=$(./ela-cli-crosschain wallet --export -p elastos | tail -2 | head -1 | cut -d' ' -f2)
    # Make sure your info is correct
    echo $ELAADDRESS $PUBLICKEY $PRIVATEKEY
    ```

4. Transfer ELA from the resources wallet to this newly created wallet

    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": '"$ELAADDRESS"',"amount": "10100"}]}' localhost:8091/api/1/transfer
    ```

    Check whether the ELA got transferred successfully

    ```
    ./ela-cli-crosschain wallet -l
    ```
5. Transfer ELA from main chain to token sidechain

    ```
    ./ela-cli-crosschain wallet -t create --from $ELAADDRESS --deposit 0x4505b967d56f84647eb3a40f7c365f7d87a88bc3 --amount 2000 --fee 0.1;
    ./ela-cli-crosschain wallet -t sign -p elastos --file to_be_signed.txn;
    ./ela-cli-crosschain wallet -t send --file ready_to_send.txn;

    ./ela-cli-crosschain wallet -t create --from $ELAADDRESS --deposit 0x268b7f52010cbbca2d910b5e67260fc119afa5c9 --amount 2000 --fee 0.1;
    ./ela-cli-crosschain wallet -t sign -p elastos --file to_be_signed.txn;
    ./ela-cli-crosschain wallet -t send --file ready_to_send.txn;
    ```

- Deploy oracle service contract 
```node deployctrt.js```
