package spv

import (
	"time"

	. "SPVWallet/core"
	. "SPVWallet/msg"
)

type request struct {
	hash       Uint256
	reqType    uint8
	retryTimes int
	doneChan   chan byte
	onTimeout  func()
}

func (r *request) start() {
	r.doneChan = make(chan byte)
	go r.send()
}

func (r *request) send() {
	spv.pm.GetSyncPeer().Send(NewDataReq(r.reqType, r.hash))
	timer := time.NewTimer(time.Second * RequestTimeout)
	select {
	case <-timer.C:
		if r.retryTimes >= MaxRetryTimes {
			timer.Stop()
			r.onTimeout()
			break
		}
		r.retryTimes++
		r.send()
	case <-r.doneChan:
		timer.Stop()

	}
}

func (r *request) finish() {
	r.doneChan <- 1
}
