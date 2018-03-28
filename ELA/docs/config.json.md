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
    "HttpInfoStart": true,  //ture to start the webUI, false to disable
    "HttpRestPort": 10334,  //Restful port number
    "HttpWsPort": 10335,    //Websocket port number
    "WsHeartbeatInterval": 60,
    "HttpJsonPort": 10336,  //RPC port number
    "NoticeServerUrl": "",  //
    "OauthServerUrl": "",
    "NodePort": 10338,      //P2P port number
    "PrintLevel": 1,        //Log level. Level 0 is the highest, 6 is the lowest.
    "IsTLS": false,         //TLS connection, true or false
    "CertPath": "./sample-cert.pem",  //Certificate path
    "KeyPath": "./sample-cert-key.pem",
    "CAPath": "./sample-ca.pem",
    "MultiCoreNum": 4,      //Max number of CPU cores to mine ELA
    "MaxTransactionInBlock": 10000, //Max transaction number in each block
    "MaxBlockSize": 8000000,        //Max size of a block
    "ConsensusType": "pow",         //
    "PowConfiguration": {           //
      "PayToAddr": "",              //Pay bonus to this address. Cannot be empty if AutoMining set to "true". 
      "MiningServerIP": "127.0.0.1",  //Miner server address for mining pool usage. By detault set to the current node. No need to change without mining pool.
      "MiningServerPort": 5555,     //Not in use so far
      "MiningSelfPort": 10339,      //
      "TestNet": true,              //No in use so far
      "AutoMining": false,          //Start mining automatically? true or false
      "MinerInfo": "ELA",           //No need to change.
      "MinTxFee": 100,              //Minimal mining fee
      "ActiveNet": "MainNet"        //Network type. Choices: MainNet、TestNet、RegNet，RegNet. Mining interval are 120s、10s、1s accordingly. Difficulty factor high to low.
    }
  }
}

```
