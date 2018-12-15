# Config.json explanation

## In most cases, you only need to modify
- NetType
- AutoMining、
- PayToAddr、
- Magic、
- SeedList、
- HttpInfoPort、
- HttpRestPort、
- HttpWsPort、
- HttpJsonPort、
- NodePort，
- InstantBlock  //for reduce the mining blocks interval

## Inline Explanation

```
{
  "NetType": "MainNet",//Network type. Choices: MainNet、TestNet
  "Configuration": {
    "Magic": 20180312,      //Magic Number：Segregation for different subnet. No matter the port number, as long as the magic number not matching, nodes cannot talk to each others.
    "SpvMagic":2017001,
    "Version": 1,          //Version number
    "SeedList": [           //SeedList. Other nodes will look up this seed list to connect to any of those seed in order to get all nodes addresses.
      "127.0.0.1:10338",    //At least one seed in this list. Format is "IP address : Port"
      "127.0.0.1:20338",
      "127.0.0.1:30338",
      "127.0.0.1:40338"
    ],
    "SpvSeedList":[
    ],
    "SpvMinOutbound":3,
    "SpvMaxConnections":10,
    "MainChainDefaultPort":10866,	
    "ExchangeRate":1.0,
    "MinCrossChainTxFee": 10000,    //Minimal cross-chain transaction fee
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
  	 "NoticeServerUrl":"",
  	 "PrintSyncState":true,         //Whether to print synchronous block height
  	 "ConsensusType":"pow",
  	 "MainChainFoundationAddress":"EM8DhdWEFmuLff9fH7fZssK7h5ayUzKcV7",
  	 "FoundationAddress":"EPwPux7M4YQZyhJbGsZzCUSdkEby3s8uYJ",
    "PowConfiguration": {           //
      "PayToAddr": "",              //Pay bonus to this address. Cannot be empty if AutoMining set to "true".
      "AutoMining": false,          //Start mining automatically? true or false
      "MinerInfo": "ELA",           //No need to change.
      "MinTxFee": 100,              //Minimal mining fee
      "InstantBlock": false        //for reduce the mining blocks interval
    }
  }
}

```
