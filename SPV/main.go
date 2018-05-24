package main

import (
	"os"
	"os/signal"
	"encoding/binary"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
	"github.com/elastos/Elastos.ELA.SPV/log"
)

func main() {
	// Initiate log
	log.Init(config.Values().PrintLevel)

	file, err := spvwallet.OpenKeystoreFile()
	if err != nil {
		log.Error("Keystore.dat file not found, please create your wallet using ela-wallet first")
		os.Exit(0)
	}

	// Initiate SPV service
	iv, _ := file.GetIV()
	wallet, err := spvwallet.Init(binary.LittleEndian.Uint64(iv), config.Values().SeedList)
	if err != nil {
		log.Error("Initiate SPV service failed,", err)
		os.Exit(0)
	}

	// Handle interrupt signal
	stop := make(chan int, 1)
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		for range c {
			log.Trace("SPVWallet shutting down...")
			wallet.Stop()
			stop <- 1
		}
	}()

	wallet.Start()

	<-stop
}
