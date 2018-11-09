package main

import (
	"os"

	"github.com/elastos/Elastos.ELA.Utility/signal"
)

func main() {
	// Listen interrupt signals.
	interrupt := signal.NewInterrupt()

	// Create the SPV wallet instance.
	w, err := NewWallet()
	if err != nil {
		waltlog.Error("Initiate SPV service failed,", err)
		os.Exit(0)
	}
	defer w.Stop()

	w.Start()

	<-interrupt.C
}
