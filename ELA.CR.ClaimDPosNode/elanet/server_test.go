package elanet

import (
	"testing"
	"time"
)

func TestPeerMsg(t *testing.T) {
	p1 := newPeerMsg{nil}
	p2 := donePeerMsg{nil}

	peers := make(chan interface{}, 2)
	peers <- p1
	peers <- p2

	go func() {
		for p := range peers {
			switch p.(type) {
			case newPeerMsg:
				t.Log("newPeerMsg")
			case donePeerMsg:
				t.Log("donePeerMsg")
			default:
				t.Log("unknown msg")
			}
		}
	}()

	<-time.After(time.Millisecond)
}
