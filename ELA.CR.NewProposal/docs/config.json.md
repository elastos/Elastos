# Config.json explanation

## In most cases, you only need to modify

- AutoMining、
- PayToAddr、
- Magic、
- SeedList、
- HttpInfoPort、
- HttpRestPort、
- HttpWsPort、
- HttpJsonPort、
- NodePort，
- ActiveNet  //for reduce the mining blocks interval
- RpcConfiguration //for limit ip to use rpc interface
## Inline Explanation

```JSON
{
  "Configuration": {
    "Magic": 20180312,      //Magic Number：Segregation for different subnet. No matter the port number, as long as the magic number not matching, nodes cannot talk to each others.
    "Version": 23,          //Version number
    "SeedList": [           //SeedList. Other nodes will look up this seed list to connect to any of those seed in order to get all nodes addresses.
      "127.0.0.1:10338",    //At least one seed in this list. Format is "IP address : Port"
      "127.0.0.1:20338",
      "127.0.0.1:30338",
      "127.0.0.1:40338"
    ],
    "HttpInfoPort": 10333,  //Local web portal port number. User can go to http://127.0.0.1:10333/info to access the web UI
    "HttpInfoStart": true,  //true to start the webUI, false to disable
    "HttpRestPort": 10334,  //Restful port number
    "HttpWsPort": 10335,    //Websocket port number
    "WsHeartbeatInterval": 60,
    "HttpJsonPort": 10336,  //RPC port number
    "NodePort": 10338,      //P2P port number
    "NodeOpenPort": 10866,  //P2P port number for open service
    "OpenService": true,    //true to enable open service, false to disable
    "PrintLevel": 1,        //Log level. Level 0 is the highest, 6 is the lowest.
    "MaxLogsSize": 5000,    //Max total logs size in MB
    "MaxPerLogSize": 20,    //Max per log file size in MB
    "IsTLS": false,         //TLS connection, true or false
    "CertPath": "./sample-cert.pem",  //Certificate path
    "KeyPath": "./sample-cert-key.pem",
    "CAPath": "./sample-ca.pem",
    "MultiCoreNum": 4,      //Max number of CPU cores to mine ELA
    "MaxTransactionInBlock": 10000, //Max transaction number in each block
    "MaxBlockSize": 8000000,        //Max size of a block
    "MinCrossChainTxFee": 10000,    //Minimal cross-chain transaction fee
    "PowConfiguration": {           //
      "PayToAddr": "",              //Pay bonus to this address. Cannot be empty if AutoMining set to "true".
      "AutoMining": false,          //Start mining automatically? true or false
      "MinerInfo": "ELA",           //No need to change.
      "MinTxFee": 100,              //Minimal mining fee
      "ActiveNet": "MainNet"        //Network type. Choices: MainNet、TestNet、RegNet，RegNet. Mining interval are 120s、10s、1s accordingly. Difficulty factor high to low.
    },
    "VoteHeight": 100000,           //Starting height of statistical voting
    "RpcConfiguration": {           
      "User": "ELAUser",            //User name: if set, you need to provide user name and password when calling the rpc interface
      "Pass": "ELAPass" ,           //User password: if set, you need to provide user name and password when calling the rpc interface
      "WhiteIPList":[               //If hanve "0.0.0.0" in WhiteIPList will allow all ip to connect, otherwise only allow ip in WhiteIPList to connect
        "0.0.0.0"
      ]
    },
    "Arbiters": [          //Public keys of the arbitrator nodes, used to verify cross-chain transfer transactions and sidechain blocks
      "03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
      "02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
      "03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
      "03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
      "02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448"
    ]
  }
}

```
