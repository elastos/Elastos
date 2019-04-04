package signal

import (
	"os"
	"os/signal"
)

type interrupt struct {
	C chan struct{}
}

// Interrupted returns if interrupt signals has received.
func (i *interrupt) Interrupted() bool {
	select {
	case <-i.C:
		return true
	default:
	}
	return false
}

// NewListener returns a signal listener instance.  It listens for OS Signals
// such as SIGINT (Ctrl+C) and SIGKILL (kill -9).
func NewInterrupt() *interrupt {
	i := interrupt{
		C:make(chan struct{}),
	}
	go func() {
		signals := make(chan os.Signal, 1)
		signal.Notify(signals, os.Interrupt, os.Kill)

		// Listen for initial shutdown signal and close the returned
		// channel to notify the caller.
		select {
		case <-signals:
			close(i.C)
		}

		// Listen for repeated signals.
		for {
			select {
			case <-signals:
			}
		}
	}()
	return &i
}
