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

func getCRCArbiters() []config.CRCArbiter {
	crcs := []struct {
		PublicKey  string
		NetAddress string
	}{
		{
			PublicKey:  "02eae9164bd143eb988fcd4b7a3c9c04a44eb9a009f73e7615e80a5e8ce1e748b8",
			NetAddress: "127.0.0.1:10078",
		},
		{
			PublicKey:  "0294d85959f746b8e6e579458b41eea05afeae50f5a37a037de601673cb24133d9",
			NetAddress: "127.0.0.1:10178",
		},
		{
			PublicKey:  "03b0a3a16edfba8d9c1fed9094431c9f24c78b8ceb04b4b6eeb7706f1686b83499",
			NetAddress: "127.0.0.1:10278",
		},
		{
			PublicKey:  "0222461ae6c9671cad288f10469f9fd759912f257c64524367dc12c40c2bb4046d",
			NetAddress: "127.0.0.1:10378",
		},
	}

	arbiters := make([]config.CRCArbiter, 0, len(crcs))
	for _, crc := range crcs {
		arbiters = append(arbiters, config.CRCArbiter{
			PublicKey:  crc.PublicKey,
			NetAddress: crc.NetAddress,
		})
	}

	return arbiters
}

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
	srvrlog := backend.Logger("SRVR", elalog.LevelDebug)
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

	chainParams := &config.DefaultParams
	chainParams.VoteStartHeight = 1
	chainParams.CRCOnlyDPOSHeight = 250
	chainParams.PublicDPOSHeight = 280
	chainParams.PreConnectOffset = 20
	chainParams.OriginArbiters = []string{
		"02f3876d0973210d5af7eb44cc11029eb63a102e424f0dc235c60adb80265e426e",
		"03c96f2469b43dd8d0e6fa3041a6cee727e0a3a6658a9c28d91e547d11ba8014a1",
		"036d25d54fb7a40bc7c3e836a26c9e30d5294bc46f6918ad61d0937960f13307bc",
		"0248ddc9ac60f1e5b9e9a26719a8a20e1447e6f2bbb0d31597646f1feb9704f291",
		"02e34e47a06955ef1ec0d325c9edada34a0df6e519530344cc85f5942d061223b3",
	}
	chainParams.CRCArbiters = getCRCArbiters()

	seedList := []string{
		"127.0.0.1:10338",
		"127.0.0.1:10018",
		"127.0.0.1:10118",
		"127.0.0.1:10218",
		"127.0.0.1:10318",
		"127.0.0.1:10418",
		"127.0.0.1:10518",
		"127.0.0.1:10618",
		"127.0.0.1:10718",
		"127.0.0.1:10058",
	}

	cfg := DPOSConfig{
		Config: Config{
			DataDir:        "test",
			Magic:          7630401,
			Foundation:     "EM8DhdWEFmuLff9fH7fZssK7h5ayUzKcV7",
			SeedList:       seedList,
			DefaultPort:    22866,
			MinOutbound:    8,
			MaxConnections: 100,
		},
		ChainParams: chainParams,
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
