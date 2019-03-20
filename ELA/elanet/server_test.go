package elanet

import (
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/elanet/peer"
	svr "github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/stretchr/testify/assert"
)

func TestPeerMsg(t *testing.T) {
	p := &peer.Peer{}
	p1 := newPeerMsg{p}
	p2 := donePeerMsg{p}

	peers := make(chan interface{}, 2)
	peers <- p1
	peers <- p2

	go func() {
		set := make(map[svr.IPeer]struct{})
		for p := range peers {
			switch p := p.(type) {
			case newPeerMsg:
				set[p.IPeer] = struct{}{}
			case donePeerMsg:
				_, ok := set[p.IPeer]
				assert.Equal(t, true, ok)
			default:
				t.FailNow()
			}
		}
	}()

	<-time.After(time.Millisecond)
}
