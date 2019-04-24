### Build Wallet and DID Service 
```bash
mvn clean; mvn install -Dmaven.test.skip -Dgpg.skip
```

### Some useful info
```bash
DID Genesis: XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ
Token Genesis: XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4

Foundation: ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw
Mining: EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U
```

### Prepare Staging Environment
```bash
repo=Elastos.ELA
repo2=ela
docker_file=Dockerfile-ela
binary=ela

repo=Elastos.ELA.Arbiter
repo2=ela-arbiter
docker_file=Dockerfile-ela-arbiter
binary=arbiter

repo=Elastos.ELA.SideChain.ID
repo2=ela-did
docker_file=Dockerfile-did
binary=did

repo=Elastos.ELA.SideChain.Token
repo2=ela-token
docker_file=Dockerfile-token
binary=token

repo=Elastos.ORG.API.Misc
repo2=api-misc
docker_file=Dockerfile-ela-api
binary=misc

rm -rf ~/dev/src/github.com/kpachhai/ela-privnet-staging/$repo2/*; cp -r ~/dev/src/github.com/elastos/$repo/* ~/dev/src/github.com/kpachhai/ela-privnet-staging/$repo2/.
docker build -t test -f $docker_file .
container_id=$(docker run -it -d test)
docker cp $container_id:/go/src/github.com/elastos/$repo/$binary ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/
```

### Copy from docker container to host
```
docker cp ela-mainchain-normal-1:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_normal/mainchain-normal-1/
docker cp ela-mainchain-dpos-1:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_dpos/mainchain-dpos-1/
docker cp ela-mainchain-dpos-2:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_dpos/mainchain-dpos-2/
docker cp ela-mainchain-crc-1:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_crc/mainchain-crc-1/
docker cp ela-mainchain-crc-2:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_crc/mainchain-crc-2/
docker cp ela-mainchain-crc-3:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_crc/mainchain-crc-3/
docker cp ela-mainchain-crc-4:/home/elauser/elastos ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain/node_crc/mainchain-crc-4/

docker cp ela-arbitrator-origin-1:/home/elauser/elastos_arbiter ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-arbitrator/node_origin/arbitrator-origin-1/
docker cp ela-arbitrator-origin-2:/home/elauser/elastos_arbiter ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-arbitrator/node_origin/arbitrator-origin-2/
docker cp ela-arbitrator-crc-1:/home/elauser/elastos_arbiter ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-arbitrator/node_crc/arbitrator-crc-1/
docker cp ela-arbitrator-crc-2:/home/elauser/elastos_arbiter ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-arbitrator/node_crc/arbitrator-crc-2/
docker cp ela-arbitrator-crc-3:/home/elauser/elastos_arbiter ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-arbitrator/node_crc/arbitrator-crc-3/
docker cp ela-arbitrator-crc-4:/home/elauser/elastos_arbiter ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-arbitrator/node_crc/arbitrator-crc-4/

docker cp ela-sidechain-did-1:/home/elauser/elastos_did ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/did/did-1/
docker cp ela-sidechain-did-2:/home/elauser/elastos_did ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/did/did-2/
docker cp ela-sidechain-did-3:/home/elauser/elastos_did ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/did/did-3/
docker cp ela-sidechain-did-4:/home/elauser/elastos_did ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/did/did-4/

docker cp ela-sidechain-token-1:/home/elauser/elastos_token ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/token/token-1/
docker cp ela-sidechain-token-2:/home/elauser/elastos_token ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/token/token-2/
docker cp ela-sidechain-token-3:/home/elauser/elastos_token ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/token/token-3/
docker cp ela-sidechain-token-4:/home/elauser/elastos_token ~/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-sidechain/token/token-4/
```

### Distribute ELA to appropriate addresses in main chain and sidechains
```bash
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw","privateKey": "79aaa0b2df79e82e687063ff7fd03579130095ec93aa2d1861ee669aabff69c3"}],"receiver": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","amount": "16000000"}]}' localhost:8091/api/1/transfer
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr","amount": "100000"}]}' localhost:8091/api/1/transfer
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY","amount": "100000"}]}' localhost:8091/api/1/transfer
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3","amount": "100000"}]}' localhost:8091/api/1/cross/m2d/transfer
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3"}}' http://localhost:10034
./ela-cli wallet -t create --from EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s --deposit EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW --amount 100000 --fee 0.0001; ./ela-cli wallet -t sign -p 123 --file to_be_signed.txn; ./ela-cli wallet -t send --file ready_to_send.txn
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW"}}' http://localhost:10044
```

### Send ELA to wallets that will be used in registering for supernodes
```bash
Private net wallet adress 1 that needs ELA: EPwYwLCrxiANe87Qhw35vsrHR95sDR6ANo
Owner's Public Key: 02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EPwYwLCrxiANe87Qhw35vsrHR95sDR6ANo","amount": "100000"}]}' localhost:8091/api/1/transfer

Private net wallet address 2 that needs ELA: EZb2xME3AprQXPv17Xcq1C6qW8dxN2Jt17
Owner's Public Key: 03349e33ed837402a2d54df3c73e7b6146531c96113f5f6eb4dfed3392d0ba227c
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EZb2xME3AprQXPv17Xcq1C6qW8dxN2Jt17","amount": "100000"}]}' localhost:8091/api/1/transfer
```

### Caste vote to supernodes
NOTE: You gotta use owner's public key here. Also, as soon as ELA is taken out of the sender address, the votes are reset
```bash
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","amount": "50000","candidatePublicKeys":["02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b","03349e33ed837402a2d54df3c73e7b6146531c96113f5f6eb4dfed3392d0ba227c"]}]}' localhost:8091/api/1/dpos/vote
```

### Verify that the supernodes are in fact registered and have received some votes
```bash
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params":{"start":"0","limit":2}}' http://localhost:10014
```