package main

import (
	"os"
	"fmt"

	"encoding/binary"
	"SPVWallet/db"
	"SPVWallet/p2p"
	"SPVWallet/wallet"
	"SPVWallet/log"
)

/*
The SPV wallet program, running background
communication with node through web socket interface

First of all, check if main account was created. If not, show hint message and exit.
Then sync block headers to best height, web socket will not start until blocks were synchronized.
Then connect to node web socket interface and register wallet accounts,
receive blocks and transactions of the registered wallet accounts from web socket message push,
verify transactions with block headers and update accounts balance.
*/

func main() {
	// Initiate log
	log.Init()

	file, err := wallet.OpenKeystoreFile()
	if err != nil {
		fmt.Println("Keystore.dat file not found, please create your wallet using wallet-cli first")
		os.Exit(0)
	}

	// Initiate blockchain
	db.InitBlockchain()

	// Initiate P2P network
	iv, _ := file.GetIV()
	p2p.Init(binary.LittleEndian.Uint64(iv))

	// Start SPV wallet
	p2p.Start()
}
