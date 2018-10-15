package main

import (
	"os"
	"os/signal"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet"
)

func main() {
	// Initiate SPV service
	wallet, err := spvwallet.New()
	if err != nil {
		waltlog.Error("Initiate SPV service failed,", err)
		os.Exit(0)
	}

	// Handle interrupt signal
	stop := make(chan int, 1)
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		for range c {
			waltlog.Trace("Wallet shutting down...")
			wallet.Stop()
			stop <- 1
		}
	}()

	wallet.Start()

	<-stop
}
