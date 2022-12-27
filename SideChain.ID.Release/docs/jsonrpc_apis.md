Instructions
===============

this is the document of ela json rpc interfaces.
it follows json-rpc 2.0 protocol but also keeps compatible with 1.0 version. 
That means both named params and positional params are acceptable.

"id" is optional, which will be sent back in the result samely if you add it in a request. 
It is needed when you want to distinguish different requests.

"jsonrpc" is optional. It tells which version this request uses.
In version 2.0 it is required, while in version 1.0 it does not exist.

#### getidentificationtxbyidandpath

description: get registered id transaction by id and path
parameters:

| name | type   | description            |
| ---- | ------ | ---------------------- |
| id   | string | id of identification   |
| path | string | path of identification |

results: registered id transaction information
argument sample:

```json
{
	"method": "getidentificationtxbyidandpath",
	"params":{
		"id":"igRn4VtAUB7hPkMrYMHt5f3xiQUMQJUFD2",
		"path": "kyc/person/identityCard"
	}
}
```

result sample:

```json
{
  "result": {
    "txid":"277f428f0be9f60bf3ba996540f3a4b467ac75f1296d41b5543edcc3190d944e",
    "hash":"277f428f0be9f60bf3ba996540f3a4b467ac75f1296d41b5543edcc3190d944e",
    "size":733,
    "vsize":733,
    "version":0,
    "locktime":0,
    "vin":[
        {
            "txid":"860b98c591d88c2eeaea521c32d35f191e1d039378c58bf47bbaf7752ecaa9ca",
            "vout":0,
            "sequence":4294967295
        }
    ],
    "vout":[
        {
            "value":"0",
            "n":0,
            "address":"igRn4VtAUB7hPkMrYMHt5f3xiQUMQJUFD2",
            "assetid":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
            "outputlock":0
        },
        {
            "value":"98.99990000",
            "n":1,
            "address":"EfdVME9U6u1e774R4YeXpPQN3vLVsmmkee",
            "assetid":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
            "outputlock":0
        }
    ],
    "blockhash":"5ac927a80e58e4337ee791c4c3e7c0d40f54ab6b63efb11ece9630259a57dbc3",
    "confirmations":10,
    "time":1539155763,
    "blocktime":1539155763,
    "type":9,
    "payloadversion":0,
    "payload":{
        "id":"igRn4VtAUB7hPkMrYMHt5f3xiQUMQJUFD2",
        "sign":"40bc9424152e20c20909909d87036a4f54e8e30e7ecce65b235e41dac2cf0ea8954c93453e6b275963baf77ea71470123f70a83053327d071ead86315e685e564b",
        "contents":[
            {
                "path":"kyc/person/identityCard",
                "values":[
                    {
                      "datahash":"bd117820c4cf30b0ad9ce68fe92b0117ca41ac2b6a49235fabd793fc3a9413c0",
                      "proof":""signature":"30450220499a5de3f84e7e919c26b6a8543fd24129634c65ee4d38fe2e3386ec8a5dae57022100b7679de8d181a454e2def8f55de423e9e15bebcde5c58e871d20aa0d91162ff6","notary":"COOIX"",
                      "info":"information for register"
                    }
                ]
            }
        ]
    },
    "attributes":[
        {
            "usage":0,
            "data":"31353831333330393234"
        },
        {
            "usage":145,
            "data":"6d656d6f"
        }
    ],
    "programs":[
        {
            "code":"2103e1963a35418da50a0b2749c901fd246b08522e5fa192cb1f3a2de8a9785eeeefad",
            "parameter":"400f0ecff1d11fae00dbad8ab70966bb3e6e2879978dadc322a73d5e5236cf5818adacf059777a904eec8e6dd15f97bc1422d861af9e9c837a32b70d0f623970f6"
        },
        {
            "code":"21027035769801eee0fd34b76a11a251dfcde5f0e763a626e93af905c4c0d382334fac",
            "parameter":"40e024b0d2465ec851fc56db1118f05e2c083320c30610c4850aa632d0187f5ccb0d70840044f2e1daab9e677baa4fd86e569d91a1438fe0fb8c7f2974d567f4fe"
        }
    ]
  }
}
```
#### resolvedid

description: get registered id transactions' payloads by did and getall. 
If request argument getall is false then request result returns newest 
transaction's payload otherwise request result returns all this did's transactions' 
payloads

parameters:

| name | type   | description              |
| ---- | ------ | ------------------------ |
| did  | string | did of identification    |
| all  | bool   | whether get all payloads |

results: did transactions' payloads information

argument sample:
```json
{
"method": "resolvedid",
  "params":{
  	"did": "ifawgWFmZRLXN1JVmqpXcNRurhB1zyHNcf",
  	"all": false
  }
}
```

result sample when all is false:

```json
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "did": "did:elastos:ifawgWFmZRLXN1JVmqpXcNRurhB1zyHNcf",
        "status": 0,
        "transaction": [
            {
                "txid": "4a691f8b6d67f07e9352be27ffeb5c2d2e432ee3de85abeb457e4427e69f3b74",
                "timestamp": "2020-08-15T17:00:00Z",
                "operation": {
                    "header": {
                        "specification": "elastos/did/1.0",
                        "operation": "update",
                        "previousTxid": "05dd212f746831ec47f3aae674d9ab16e2a3f75de2b4c811df02568cf926e447"
                    },
                    "payload": "eyJpZCI6ICJkaWQ6ZWxhc3RvczppZmF3Z1dGbVpSTFhOMUpWbXFwWGNOUnVyaEIxenlITmNmIiwicHVibGljS2V5IjogW3siaWQiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiNtYXN0ZXIta2V5IiwidHlwZSI6ICJFQ0RTQXNlY3AyNTZyMSIsImNvbnRyb2xsZXIiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiIsInB1YmxpY0tleUJhc2U1OCI6ICJ6eHQ2Tnlvb3JGVUZNWEE4bURCVUxqbnVIM3Y2aU5kWm00MlB5RzRjMVlkQyJ9XSwiYXV0aG9yaXphdGlvbiI6IFsiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiJdLCJleHBpcmVzIjogIjIwMjAtMDgtMTVUMTc6MDA6MDBaIn0",
                    "proof": {
                        "type": "ECDSAsecp256r1",
                        "verificationMethod": "#master-key",
                        "signature": "UCi/Y3gssvbwdOu0tCmUpYEUk8n+rFbhoOUrEwdNqfcCMP5BB2qeMH5/7yl7gQKHmYyxJzP8CVj4lWBs7xw3Rw=="
                    }
                }
            }
        ]
    },
    "error": null
}
```

result sample when all is true:

```json
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "did": "did:elastos:ifawgWFmZRLXN1JVmqpXcNRurhB1zyHNcf",
        "status": 0,
        "transaction": [
            {
                "txid": "4a691f8b6d67f07e9352be27ffeb5c2d2e432ee3de85abeb457e4427e69f3b74",
                "timestamp": "2020-08-15T17:00:00Z",
                "operation": {
                    "header": {
                        "specification": "elastos/did/1.0",
                        "operation": "update",
                        "previousTxid": "05dd212f746831ec47f3aae674d9ab16e2a3f75de2b4c811df02568cf926e447"
                    },
                    "payload": "eyJpZCI6ICJkaWQ6ZWxhc3RvczppZmF3Z1dGbVpSTFhOMUpWbXFwWGNOUnVyaEIxenlITmNmIiwicHVibGljS2V5IjogW3siaWQiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiNtYXN0ZXIta2V5IiwidHlwZSI6ICJFQ0RTQXNlY3AyNTZyMSIsImNvbnRyb2xsZXIiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiIsInB1YmxpY0tleUJhc2U1OCI6ICJ6eHQ2Tnlvb3JGVUZNWEE4bURCVUxqbnVIM3Y2aU5kWm00MlB5RzRjMVlkQyJ9XSwiYXV0aG9yaXphdGlvbiI6IFsiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiJdLCJleHBpcmVzIjogIjIwMjAtMDgtMTVUMTc6MDA6MDBaIn0",
                    "proof": {
                        "type": "ECDSAsecp256r1",
                        "verificationMethod": "#master-key",
                        "signature": "UCi/Y3gssvbwdOu0tCmUpYEUk8n+rFbhoOUrEwdNqfcCMP5BB2qeMH5/7yl7gQKHmYyxJzP8CVj4lWBs7xw3Rw=="
                    }
                }
            },
            {
                "txid": "05dd212f746831ec47f3aae674d9ab16e2a3f75de2b4c811df02568cf926e447",
                "timestamp": "2020-08-15T17:00:00Z",
                "operation": {
                    "header": {
                        "specification": "elastos/did/1.0",
                        "operation": "update",
                        "previousTxid": "23df5f7d0befb743899c22357f774df3d9ce809917b07559c59f12771e67aa31"
                    },
                    "payload": "eyJpZCI6ICJkaWQ6ZWxhc3RvczppZmF3Z1dGbVpSTFhOMUpWbXFwWGNOUnVyaEIxenlITmNmIiwicHVibGljS2V5IjogW3siaWQiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiNtYXN0ZXIta2V5IiwidHlwZSI6ICJFQ0RTQXNlY3AyNTZyMSIsImNvbnRyb2xsZXIiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiIsInB1YmxpY0tleUJhc2U1OCI6ICJ6eHQ2Tnlvb3JGVUZNWEE4bURCVUxqbnVIM3Y2aU5kWm00MlB5RzRjMVlkQyJ9XSwiYXV0aG9yaXphdGlvbiI6IFsiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiJdLCJleHBpcmVzIjogIjIwMjAtMDgtMTVUMTc6MDA6MDBaIn0",
                    "proof": {
                        "type": "ECDSAsecp256r1",
                        "verificationMethod": "#master-key",
                        "signature": "r4t+DvxL7m3XiSuKoJw7k8nPMlQhhE9GPlSQJa0Tu52NOMtw+Kb8s3/bdmEahXQ3wQzBtqp42eMhakNFiiKwTA=="
                    }
                }
            },
            {
                "txid": "23df5f7d0befb743899c22357f774df3d9ce809917b07559c59f12771e67aa31",
                "timestamp": "2020-08-15T17:00:00Z",
                "operation": {
                    "header": {
                        "specification": "elastos/did/1.0",
                        "operation": "create"
                    },
                    "payload": "eyJpZCI6ICJkaWQ6ZWxhc3RvczppZmF3Z1dGbVpSTFhOMUpWbXFwWGNOUnVyaEIxenlITmNmIiwicHVibGljS2V5IjogW3siaWQiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiNtYXN0ZXIta2V5IiwidHlwZSI6ICJFQ0RTQXNlY3AyNTZyMSIsImNvbnRyb2xsZXIiOiAiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiIsInB1YmxpY0tleUJhc2U1OCI6ICJ6eHQ2Tnlvb3JGVUZNWEE4bURCVUxqbnVIM3Y2aU5kWm00MlB5RzRjMVlkQyJ9XSwiYXV0aG9yaXphdGlvbiI6IFsiZGlkOmVsYXN0b3M6aWZhd2dXRm1aUkxYTjFKVm1xcFhjTlJ1cmhCMXp5SE5jZiJdLCJleHBpcmVzIjogIjIwMjAtMDgtMTVUMTc6MDA6MDBaIn0",
                    "proof": {
                        "type": "ECDSAsecp256r1",
                        "verificationMethod": "#master-key",
                        "signature": "xiD4kHrjwmEd11RPGs3vNcb/sDQuC48PQFjCM2Yuk6YFCpjJsvY2ZWwGwlx1IUg0iqmN+4W9MkzknQa9gim+zA=="
                    }
                }
            }
        ]
    },
    "error": null
}
```