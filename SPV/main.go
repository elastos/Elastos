package main

import (
	"os"
	"os/signal"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/store"
	"github.com/elastos/Elastos.ELA.SPV/sync"
)

const LogPath = "./Logs-spv/"

func main() {
	// Initiate logger
	logger := log.NewLogger(LogPath,
		config.Values().PrintLevel,
		config.Values().MaxPerLogSize,
		config.Values().MaxLogsSize,
	)

	sdk.UseLogger(logger)
	rpc.UseLogger(logger)
	peer.UseLogger(logger)
	blockchain.UseLogger(logger)
	store.UseLogger(logger)
	sync.UseLogger(logger)
	spvwallet.UseLogger(logger)

	// Initiate SPV service
	wallet, err := spvwallet.New()
	if err != nil {
		logger.Error("Initiate SPV service failed,", err)
		os.Exit(0)
	}

	// Handle interrupt signal
	stop := make(chan int, 1)
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		for range c {
			logger.Trace("Wallet shutting down...")
			wallet.Stop()
			stop <- 1
		}
	}()

	wallet.Start()

	<-stop
}
