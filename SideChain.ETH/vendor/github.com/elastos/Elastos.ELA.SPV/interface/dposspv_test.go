package _interface

import (
	"os"
	"testing"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

// This is the DPOS SPV service test and also an example of how to use it.
func TestDPOSSPVService(t *testing.T) {
	test.SkipShort(t)

	interrupt := signal.NewInterrupt()

	backend := elalog.NewBackend(os.Stdout, elalog.Lshortfile)
	admrlog := backend.Logger("ADMR", elalog.LevelOff)
	cmgrlog := backend.Logger("CMGR", elalog.LevelOff)
	bcdblog := backend.Logger("BCDB", elalog.LevelDebug)
	synclog := backend.Logger("SYNC", elalog.LevelDebug)
	peerlog := backend.Logger("PEER", elalog.LevelDebug)
	spvslog := backend.Logger("SPVS", elalog.LevelDebug)
	srvrlog := backend.Logger("SRVR", elalog.LevelOff)
	dposlog := backend.Logger("DPOS", elalog.LevelDebug)

	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	sdk.UseLogger(spvslog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	store.UseLogger(bcdblog)
	sync.UseLogger(synclog)
	UseLogger(dposlog)

	cfg := DPOSConfig{
		Config: Config{
			DataDir:        "test",
			ChainParams:    &config.DefaultParams,
			MinOutbound:    8,
			MaxConnections: 100,
		},
		OnProducersChanged: func(sideProducerIDs [][]byte) {
			t.Logf("prodcuers changed: %v", sideProducerIDs)
		},
	}

	s, err := NewDPOSSPVService(&cfg, interrupt.C)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	s.Start()
	defer s.Stop()

	<-interrupt.C

	os.RemoveAll(cfg.DataDir)
}
