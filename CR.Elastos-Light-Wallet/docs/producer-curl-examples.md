# example curl commands

## listproducers
    curl elastos.coranos.io:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params": {"start": 0,"limit": 3}}'

## votestatus
    curl elastos.coranos.io:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"votestatus", "params": {"address":"EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda"}}'

## get already cast votes
    curl elastos.coranos.io:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getutxosbyamount", "params": {"address":"Ea8jXaAo14b3JQawqcHCCMjZ7r9GCkksZC","utxotype":"vote","amount":"0.0000001"}}'
## get mainnet tx
  curl elastos.coranos.io:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"a01c385de0c34180653362e19087d4d85fae574cc453638561167d141d0670f9","verbose":true}}' > voting/verbose-true-tx.json;

## get testnet tx
  curl elastos.coranos.io:21336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"a01c385de0c34180653362e19087d4d85fae574cc453638561167d141d0670f9","verbose":true}}' > voting/verbose-true-tx.json;
