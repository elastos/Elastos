// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

/*
DNS is a peer-to-peer network address distribute service.  It is a special
peer-to-peer server that only accept connections and send known addresses of
other peers to the connected peer.
*/
package dns

import (
	"fmt"
	"time"

	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p/server"
)

const (
	// maxPeers defines the limit of maximum connections of the DNS service.
	maxPeers = 1024

	// quickCheck is the time duration to check if a peer has finished
	// handshake.
	quickCheck = 5 * time.Second

	// peerTimeout is the max time duration we timeout and disconnect a peer.
	peerTimeout = 10 * time.Second
)

// naFilter defines a network address filter for the main chain server, for now
// it is used to filter SPV wallet addresses from relaying to other peers.
type naFilter struct{}

func (f *naFilter) Filter(na *p2p.NetAddress) bool {
	service := pact.ServiceFlag(na.Services)
	return service&pact.SFNodeNetwork == pact.SFNodeNetwork
}

// DNS is the peer-to-peer network address distribute service implement.
type DNS struct {
	server.IServer
}

// DNS service do not make any message.
func makeMessage(cmd string) (p2p.Message, error) {
	return nil, fmt.Errorf("DNS do not handle %s message", cmd)
}

// DNS service do not have a height.
func bestHeight() uint64 { return 0 }

// onNewPeer handles the new connected peer.
func onNewPeer(p server.IPeer)(bool) {
	log.Infof("New peer %s connected", p)
	go handlePeer(p.ToPeer())
	return true
}

// handlePeer waits the peer to finish request addresses and disconnect it.
func handlePeer(p *peer.Peer) {
	// Quick check the peers state after 5 seconds.  If the peer finished
	// handshake, we believe it has done request addresses and disconnect it.
	<-time.After(quickCheck)
	if p.VerAckReceived() {
		log.Infof("Disconnect peer %s through quick check", p)
		p.Disconnect()
		return
	}

	// Timeout the peer for the DNS do not keep connection with any peer, even
	// the peer do not finish request addresses.
	<-time.After(peerTimeout)
	log.Infof("Disconnect peer %s due to timeout", p)
	p.Disconnect()
}

// New creates and return a DNS instance.
func New(dataDir string, magic uint32, port uint16) (*DNS, error) {
	svrCfg := server.NewDefaultConfig(
		magic, pact.DPOSStartVersion, 0, port, nil,
		[]string{fmt.Sprintf(":%d", port)},
		nil, nil, makeMessage, bestHeight,
	)
	svrCfg.DataDir = dataDir
	svrCfg.MaxPeers = maxPeers
	svrCfg.NAFilter = &naFilter{}
	svrCfg.TargetOutbound = 0
	svrCfg.DisableRelayTx = true
	svrCfg.OnNewPeer = onNewPeer

	svr, err := server.NewServer(svrCfg)
	if err != nil {
		return nil, err
	}

	return &DNS{svr}, nil
}
