package mainchain

import (
	"os"
	"os/signal"
	"syscall"
	"time"
)

const MaxSignalQueueLen = 5

type SignalMap map[os.Signal]func()

func (s SignalMap) register(signal os.Signal, handler func()) {
	s[signal] = handler
}

func (s SignalMap) handle(signal os.Signal) {
	if handler, ok := s[signal]; ok {
		handler()
	}
}

func HandleSignal(handler func()) {
	signalList := make(SignalMap)
	signalList.register(syscall.SIGINT, handler)
	signalList.register(syscall.SIGTERM, handler)

	sigChan := make(chan os.Signal, MaxSignalQueueLen)
	signal.Notify(sigChan)

	go func() {
		for {
			select {
			case sig := <-sigChan:
				signalList.handle(sig)
			default:
				time.Sleep(time.Second)
			}
		}
	}()
}
