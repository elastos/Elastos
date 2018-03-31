package main

import (
	"os"
	"os/signal"
	"encoding/binary"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"
)

func main() {
	// Initiate log
	log.Init()

	file, err := spvwallet.OpenKeystoreFile()
	if err != nil {
		log.Error("Keystore.dat file not found, please create your wallet using ela-wallet first")
		os.Exit(0)
	}

	// Initiate SPV service
	iv, _ := file.GetIV()
	service, err := spvwallet.InitSPV(binary.LittleEndian.Uint64(iv))
	if err != nil {
		log.Error("Initiate SPV service failed,", err)
		os.Exit(0)
	}

	// Start RPC service
	server := rpc.InitServer(&rpc.Listeners{
		NotifyNewAddress: service.NotifyNewAddress,
		SendTransaction:  service.SendTransaction,
	})

	// Handle interrupt signal
	stop := make(chan int, 1)
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		for range c {
			log.Trace("SPVWallet shutting down...")
			service.Stop()
			server.Close()
			stop <- 1
		}
	}()

	service.Start()
	server.Start()

	<-stop
}
