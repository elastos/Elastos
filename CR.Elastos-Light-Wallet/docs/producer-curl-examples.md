# example curl commands

## listproducers
    curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params": {"start": 0,"limit": 3}}'

## votestatus
    curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"votestatus", "params": {"address":"EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda"}}'

## get already cast votes
    curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getutxosbyamount", "params": {"address":"Ea8jXaAo14b3JQawqcHCCMjZ7r9GCkksZC","utxotype":"vote","amount":"0.0000001"}}'

## check for tx
  https://blockchain.elastos.org/address/EHEhJcvSmM83jVDViPoEPLaG9v9HiCpvUp

## get mainnet verbose tx
  curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"7fbd76c7b63fd4e38cc9037c552b92f0248494c7540210a3b0c50f7aea0e6022","verbose":true}}' > voting/verbose-true-tx.json;

## get mainnet encoded tx
  curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"7fbd76c7b63fd4e38cc9037c552b92f0248494c7540210a3b0c50f7aea0e6022","verbose":false}}' > voting/verbose-false-tx.json;

## get mainnet verbose tx
  curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"45906ff20d983f8408dd0da34677e66750c03f04e3db01d4efb89776d5ed8ae7","verbose":true}}' > voting/three-out-verbose-true-tx.json;

## get mainnet encoded tx
  curl coranos.cc:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"45906ff20d983f8408dd0da34677e66750c03f04e3db01d4efb89776d5ed8ae7","verbose":false}}' > voting/three-out-verbose-false-tx.json;


## create offline TX
  https://walletservice.readthedocs.io/en/latest/api_guide.html#create-offline-transaction

  curl https://node1.elaphant.app/api/v1/createTx -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"inputs":["EHEhJcvSmM83jVDViPoEPLaG9v9HiCpvUp"],"outputs" : [{"addr":"EHEhJcvSmM83jVDViPoEPLaG9v9HiCpvUp","amt" :1}]}'

## get offline TX
  curl https://node1.elaphant.app/api/v1/transaction -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"data":{"method":"getrawtransaction","params":["7fbd76c7b63fd4e38cc9037c552b92f0248494c7540210a3b0c50f7aea0e6022"]}}'

## send offline TX
  curl https://node1.elaphant.app/api/v1/transaction -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"sendrawtransaction","data":"0902000100013003b046c8ff7bdfb84460f5c0909866c2f99fc97700516c9716fa093206b8013878010000000000b462fc6b3f0a532ccc44271873af4a8d64ea97f4da67dccab00f93a491ed37bc000001000000b462fc6b3f0a532ccc44271873af4a8d64ea97f4da67dccab00f93a491ed37bc01000200000002b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a318258b0000000000000000002100e7e7a87c8673c6a45b98f87bffb3be03944e550100010001210368044f3b3582000597d40c9293ea894237a88b2cd55f79a18193399937d22664b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a30000000000000000000000002100e7e7a87c8673c6a45b98f87bffb3be03944e550000000000014140415038c25bf47bbacf3c1518287af0c52e52b116eff1726647df635479a4af4c2e71d02027055cbf3ae76a782eb013454cf45d02a46d67c3aee4f71d1a03e02f2321023845038e7a3f5f3a0d014f4327b6fa5744f5224aea240de802fcfe08740c349aac"}'
