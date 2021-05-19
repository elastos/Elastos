

# Sending ELA on the Mainchain with the Service

Your back-end app would call this service via HTTP, it's an easy familiar way to do development.

In this example we assume you have `Elastos.ORG.Wallet.Service` running already locally on port 8091.

```
curl
    -X POST
    -H "Content-Type: application/json"
    -d '{
        "sender": [
            {
                "address": "ESkBFi96dJb5RHoJDhM3EqBSKnkRuEfKjR",
                "privateKey": "51D56B6B85958876EA3DC8E0BE7B8510EAB576D6F0B0CE89958FC4C90C4027B1"
            }
        ],
        "receiver": [
            {
                "address": "EWzSbtqhWtmFpwDPoqBbiR2WXBMjxbAF55",
                "amount": "1"
            }
        ]
    }' localhost:8091/api/1/transfer
```

If all goes well you should see:

```
{"result":"ce89903c6dec901c2b34a7154356ac116423c72c53078a23aa8add3cf77495e0","status":200}
```

Where the result is the transaction hash, after a minute or two you should see your transaction.

!> Note: if you just started mining ELA, the ELA is not available until after 100 confirmations.<br/>Also ELA is utxo, so if you send consecutive transactions too quickly you may run into errors too.
