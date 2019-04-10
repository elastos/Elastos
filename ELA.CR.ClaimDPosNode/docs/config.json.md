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
    "ActiveNet": "mainnet",  // Network type. Choices: mainnet testnet and regnet
    "Magic": 20180312,        // Magic Number：Segregation for different subnet. No matter the port number, as long as the magic number not matching, nodes cannot talk to each others
    "Version": 23,           // Version number
    "SeedList": [            // SeedList. Other nodes will look up this seed list to connect to any of those seed in order to get all nodes addresses
      "127.0.0.1:30338"
    ],
    "PermanentPeers": [      // PermanentPeers. Other nodes will look up this seed list to connect to any of those seed in order to get all nodes addresses, if lost connection will try to connect again
      "127.0.0.1:40338"    
    ],
    "HttpInfoPort": 20333,        // Local web portal port number. User can go to http://127.0.0.1:10333/info to access the web UI
    "HttpRestPort": 20334,        // Restful port number
    "HttpWsPort": 20335,          // Websocket port number
    "HttpJsonPort": 20336,        // RPC port number
    "NodePort": 20338,            // P2P port number
    "PrintLevel": 0,              // Log level. Level 0 is the highest, 5 is the lowest
    "MaxLogsSize": 0,             // Max total logs size in MB
    "MaxPerLogSize": 0,           // Max per log file size in MB
    "MinCrossChainTxFee": 10000,  // Minimal cross-chain transaction fee
    "PowConfiguration": {
      "PayToAddr": "",       // Pay bonus to this address. Cannot be empty if AutoMining set to "true"
      "AutoMining": true,    // Start mining automatically? true or false
      "MinerInfo": "ELA",    // No need to change
      "MinTxFee": 100,       // Minimal mining fee
      "InstantBlock": false  // false: high difficulty to mine block  true: low difficulty to mine block
    },
    "RpcConfiguration": {
      "User": "ElaUser",  // Check the username when use rpc interface, null will not check 
      "Pass": "Ela123",   // Check the password when use rpc interface, null will not check
      "WhiteIPList": [    // Check if ip in list when use rpc interface, "0.0.0.0" will not check
        "127.0.0.1"
      ]
    },
    "EnableArbiter": false,    // Normal ela node can be false, producer node need to be true
    "ArbiterConfiguration": {
      "PublicKey": "",         // The hex string of publickey
      "Magic": 20180313,        // The magic number of arbiters network
      "NodePort": 30338,       // The node prot of arbiters network
      "PrintLevel": 0,         // Log level. Level 0 is the highest, 5 is the lowest
      "SignTolerance": 5,      // The time interval of consensus in seconds
      "MaxLogsSize": 0,        // Max total logs size in MB
      "MaxPerLogSize": 0,      // Max per log file size in MB
      "OriginArbiters": [      // The publickey list of arbiters before CRCOnlyDPOSHeight
        "02f3876d0973210d5af7eb44cc11029eb63a102e424f0dc235c60adb80265e426e",
        "03c96f2469b43dd8d0e6fa3041a6cee727e0a3a6658a9c28d91e547d11ba8014a1",
        "036d25d54fb7a40bc7c3e836a26c9e30d5294bc46f6918ad61d0937960f13307bc",
        "0248ddc9ac60f1e5b9e9a26719a8a20e1447e6f2bbb0d31597646f1feb9704f291",
        "02e34e47a06955ef1ec0d325c9edada34a0df6e519530344cc85f5942d061223b3"
      ],
      "CRCArbiters": [         // The crc arbiters after CRCOnlyDPOSHeight 
        {
          "PublicKey": "02eae9164bd143eb988fcd4b7a3c9c04a44eb9a009f73e7615e80a5e8ce1e748b8",
          "NetAddress": "127.0.0.1:10078"
        },
        {
          "PublicKey": "0294d85959f746b8e6e579458b41eea05afeae50f5a37a037de601673cb24133d9",
          "NetAddress": "127.0.0.1:10178"
        },
        {
          "PublicKey": "03b0a3a16edfba8d9c1fed9094431c9f24c78b8ceb04b4b6eeb7706f1686b83499",
          "NetAddress": "127.0.0.1:10278"
        },
        {
          "PublicKey": "0222461ae6c9671cad288f10469f9fd759912f257c64524367dc12c40c2bb4046d",
          "NetAddress": "127.0.0.1:10378"
        }
      ],
      "NormalArbitratorsCount": 24,             // The count of voted arbiters
      "CandidatesCount": 72,                    // The count of candidates
      "EmergencyInactivePenalty": 50000000000,  // EmergencyInactivePenalty defines the penalty amount the emergency producer takes.
      "MaxInactiveRounds": 1440,                // MaxInactiveRounds defines the maximum inactive rounds before producer takes penalty.
      "InactivePenalty": 10000000000,           // InactivePenalty defines the penalty amount the producer takes.
      "InactiveEliminateCount": 12,             // InactiveEliminateCount defines arbitrators count should be eliminated
      "PreConnectOffset": 360                   // PreConnectOffset defines the offset blocks to pre-connect to the block producers.
    },
    "CheckAddressHeight": 88812,   //Before the height will not check that if address is ela address
    "VoteStartHeight": 88812,      //Starting height of statistical voting
    "CRCOnlyDPOSHeight": 1008812,  //The height start DPOS by CRC producers
    "PublicDPOSHeight": 1108812    //The height start DPOS by CRCProducers and voted producers 
  }
}

```