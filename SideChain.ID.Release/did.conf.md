This is the doc file of did.conf, below is a full config sample.
Usually, we don't need a did.conf to run the did side chain program,
it will run with default parameters and connect to main peer-to-peer
network.
```
{
  "ActiveNet": "MainNet", // The side chain peer-to-peer network to connect to, MainNet TestNet etc.
  "Magic": 2018001, // Overwrite the magic number of the side chain peer-to-peer network.
  "DefaultPort": 20608, // Overwrite the default side chain peer-to-peer port.
  "SeedList": [ // Overwrite the seed peers of side chain peer-to-peer network.
    "did-mainnet-001.elastos.org",
    "did-mainnet-002.elastos.org",
    "did-mainnet-003.elastos.org",
    "did-mainnet-004.elastos.org",
    "did-mainnet-005.elastos.org"
  ],
  "SpvConfig": { // The configuration for SPV module.
    "Magic": 2017001, // Overwrite the magic number of the main chain peer-to-peer network.
    "SeedList": [ // Overwrite the seed peers of main chain peer-to-peer network.
      "node-mainnet-002.elastos.org",
      "node-mainnet-003.elastos.org",
      "node-mainnet-004.elastos.org",
      "node-mainnet-006.elastos.org",
      "node-mainnet-007.elastos.org"
    ],
    "DefaultPort": 20866, // Overwrite the default main chain peer-to-peer port.
    "Foundation": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta" // Overwrite the foundation address of main chain.
  },
  "Foundation": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta", // Overwrite the foundation address of side chain.
  "MinTxFee": 100, // Overwrite the minimum transaction fee of side chain.
  "ExchangeRate": 1, // Set the exchange rate of the cross chain transactions.
  "DisableTxFilters": false, // Set whether or not support transaction filtering.
  "MinCrossChainTxFee": 10000, // Set the minimum cross chain transaction fee.
  "Mining": false, // Set if trun on or off mining(generate) blocks.
  "MinerInfo": "DID", // The miner info will be included into the mined(generated) blocks.
  "MinerAddr": "fill this with miner address", // The address to receive reward in mined(generated) blocks.
  "LogLevel": 1, // Set log level, Debug(0), Info(1), Warn(2), Error(3), Fatal(4), Off(>=5).
  "MaxLogsFolderSize": 2000000000, // Set maximum size of all log files under log file path, default is 2GB.
  "MaxPerLogFileSize": 20000000, // Set maximum size of a single log file, default is 20MB.
  "HttpInfoStart": true, // Start info service or not, if started a webpage displaying did status can be access through HttpInfoPort.
  "HttpInfoPort": 30603, // Set the port to access did info webpage.
  "HttpRestPort": 30604, // Set the port to access did RESTful interfaces.
  "HttpJsonPort": 30606, // Set the port to access did JSON-RPC interfaces.
  "PrintSyncState": false // Set if print out block sync status or not.
}
```