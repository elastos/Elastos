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