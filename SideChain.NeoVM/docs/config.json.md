# config.json explanation

## Configuration free
- `config.json` file is optional, you can run a `neo` node without a `config.json` file.

## Change active network
Just modify the `ActiveNet` parameter in the `config.json` file.
```json
{
  "ActiveNet": "test"
}
```
Default config for `testnet`
- Peer-to-Peer network connect to ELA `testnet`.

## Inline Explanation

```json5
{
  "ActiveNet": "mainnet", // Network type. Choices: mainnet testnet and regnet
  "Magic": 2017002,       // Magic number：Segregation for different subnet. No matter the port number, as long as the magic number not matching, nodes cannot talk to each others
  "NodePort": 20608,      // The port number for P2P network connection.
  "DisableDNS": false,    // Disable the DNS service of the P2P network.
  "PermanentPeers": [     // Specify a list of peers to connect with permanently.
    "localhost:20608"
  ],
  "SPVMagic": 2017001,    // SPV magic number.
  "SPVDisableDNS": false, // Disable the DNS service of the SPV P2P network.
  "SPVPermanentPeers": [  // Specify a list of SPV peers to connect with permanently.
    "localhost:20338"
  ],
  "ExchangeRate": 1.0,    // Defines the exchange rate of main/side asset.
  "MinCrossChainTxFee": 10000, // Defines the minimum fee for a cross chain transaction.
  "EnableREST": false,    // Enable the RESTful service.
  "RESTPort": 20604,      // Specify a port for the RESTful service.
  "EnableWS": false,      // Enable the WebSocket service.
  "WSPort": 20605,        // Specify a port for the WebSocket service.
  "EnableRPC": true,      // Enable the JSON-RPC service.
  "RPCPort": 20606,       // Specify a port for the JSON-RPC service.
  "RPCUser": "User",      // Specify the username when accessing the JSON-RPC service.
  "RPCPass": "Password",  // Specify the password when accessing the JSON-RPC service.
  "RPCWhiteList": [       // Specify a white list of IP addresses for the JSON-RPC service.
    "127.0.0.1"
  ],
  "LogLevel": 1,          // Specify the debug log level. Choices: (0)debug (1)info (2)warn (3)error (4)fatal (5)off.
  "PerLogFileSize": 20,   // Specify the maximum file size of a single log file.
  "LogsFolderSize": 2048, // Specify the maximum logs folder size which containing all log files.
  "DisableTxFilters": false, // Disable the transaction filer function.
  "EnableMining": false,  // Enable the mining service which can produce blocks.
  "InstantBlock": false,  // Set the block producing to instant mode which can make the mining service produce block instantly.
  "PayToAddr": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta", // Specify the account address to receive rewards by mining blocks.
  "MinerInfo": "ELA",     // The miner info displaying the miner in coinbase transaction.
}
```
