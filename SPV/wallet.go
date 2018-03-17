package main

import (
	"os"
	"encoding/binary"
	"SPVWallet/wallet"
	"SPVWallet/log"
	"SPVWallet/p2p"
	"os/signal"
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
		log.Error("Keystore.dat file not found, please create your wallet using wallet-cli first")
		os.Exit(0)
	}

	// Initiate SPV service
	iv, _ := file.GetIV()
	spv, err := wallet.InitSPV(binary.LittleEndian.Uint64(iv))
	if err != nil {
		log.Error("Initiate SPV service failed,", err)
		os.Exit(0)
	}

	// Init listeners
	p2p.SetListeners(&p2p.Listeners{
		OnVersion:     spv.OnVersion,
		OnVerAck:      spv.OnVerAck,
		OnPing:        spv.OnPing,
		OnPong:        spv.OnPong,
		OnAddrs:       spv.OnAddrs,
		OnAddrsReq:    spv.OnAddrsReq,
		OnInventory:   spv.OnInventory,
		OnMerkleBlock: spv.OnMerkleBlock,
		OnTxn:         spv.OnTxn,
		OnNotFound:    spv.OnNotFound,
		OnDisconnect:  spv.OnDisconnect,
	})

	// Handle interrupt signal
	stop := make(chan int, 1)
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		for range c {
			log.Trace("SPVWallet shutting down...")
			spv.Stop()
			stop <- 1
		}
	}()

	spv.Start()

	<-stop
}
