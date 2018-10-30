package main

import (
	"os"
	"os/signal"
)

func main() {
	// Initiate SPV service
	w, err := NewWallet()
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
			waltlog.Debug("Wallet shutting down...")
			w.Stop()
			stop <- 1
		}
	}()

	w.Start()

	<-stop
}
