package _interface

import (
	"math/rand"
	"os"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/connmgr"
	"github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

type TxListener struct {
	t       *testing.T
	log     elalog.Logger
	service SPVService
	address string
	txType  types.TxType
	flags   uint64
}

func (l *TxListener) Address() string {
	return l.address
}

func (l *TxListener) Type() types.TxType {
	return l.txType
}

func (l *TxListener) Flags() uint64 {
	return l.flags
}

func (l *TxListener) Notify(id common.Uint256, proof bloom.MerkleProof, tx types.Transaction) {
	l.log.Infof("Notify Type %s, TxID %s", tx.TxType.Name(), tx.Hash())
	err := l.service.VerifyTransaction(proof, tx)
	if !assert.NoError(l.t, err) {
		l.t.FailNow()
	}

	txIds, err := l.service.GetTransactionIds(proof.Height)
	if !assert.NotNil(l.t, tx) {
		l.t.FailNow()
	}
	if !assert.NoError(l.t, err) {
		l.t.FailNow()
	}

	for _, txId := range txIds {
		tx, err := l.service.GetTransaction(txId)
		if !assert.NotNil(l.t, tx) {
			l.t.FailNow()
		}
		if !assert.NoError(l.t, err) {
			l.t.FailNow()
		}
	}

	// Submit transaction receipt
	l.service.SubmitTransactionReceipt(id, tx.Hash())
}

func TestGetListenerKey(t *testing.T) {
	var key1, key2 common.Uint256
	listener := &TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  types.CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	}

	key1 = getListenerKey(listener)
	key2 = getListenerKey(&TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  types.CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	})
	if !key1.IsEqual(key2) {
		t.Errorf("listeners with same fields get different key1 %s, key2 %s", key1.String(), key2.String())
	}

	// same type, flags different address
	key1 = getListenerKey(listener)
	listener.address = "Ef2bDPwcUKguteJutJQCmjX2wgHVfkJ2Wq"
	key2 = getListenerKey(listener)
	if key1.IsEqual(key2) {
		t.Errorf("listeners with different address got same key %s", key1.String())
	}

	// same address, flags different type
	key1 = getListenerKey(listener)
	listener.txType = types.TransferAsset
	key2 = getListenerKey(listener)
	if key1.IsEqual(key2) {
		t.Errorf("listeners with different type got same key %s", key1.String())
	}

	// same address, type different flags
	key1 = getListenerKey(listener)
	listener.flags = FlagNotifyInSyncing
	key2 = getListenerKey(listener)
	key2 = getListenerKey(listener)
	if key1.IsEqual(key2) {
		t.Errorf("listeners with different flags got same key %s", key1.String())
	}
}

func TestNewSPVService(t *testing.T) {
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
	listlog := backend.Logger("RPCS", elalog.LevelDebug)

	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	sdk.UseLogger(spvslog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	store.UseLogger(bcdblog)
	sync.UseLogger(synclog)

	cfg := &Config{
		ChainParams: config.DefaultParams.TestNet(),
	}

	service, err := NewSPVService(cfg)
	if err != nil {
		t.Errorf("NewSPVService error %s", err.Error())
	}

	confirmedListener := &TxListener{
		t:       t,
		log:     listlog,
		service: service,
		address: "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",
		txType:  types.CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	}

	unconfirmedListener := &TxListener{
		t:       t,
		log:     listlog,
		service: service,
		address: "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",
		txType:  types.TransferAsset,
		flags:   0,
	}

	// Set on transaction confirmed callback
	service.RegisterTransactionListener(confirmedListener)
	service.RegisterTransactionListener(unconfirmedListener)

	// Start spv service
	service.Start()

	syncTicker := time.NewTicker(time.Second * 10)
	defer syncTicker.Stop()

out:
	for {
		select {
		case <-interrupt.C:
			break out

		case <-syncTicker.C:

			best, err := service.headers.GetBest()
			if !assert.NoError(t, err) {
				t.FailNow()
			}

			height := uint32(rand.Int31n(int32(best.Height)))

			t.Logf("GetBlock from height %d", height)
			_, err = service.headers.GetByHeight(height)
			if !assert.NoError(t, err) {
				t.FailNow()
			}

			t.Logf("GetTransactionIds from height %d", height)
			txIds, err := service.GetTransactionIds(height)
			if !assert.NoError(t, err) {
				t.FailNow()
			}

			for _, txId := range txIds {
				tx, err := service.GetTransaction(txId)
				if !assert.NotNil(t, tx) {
					t.FailNow()
				}
				if !assert.NoError(t, err) {
					t.FailNow()
				}
			}

			if service.IService.IsCurrent() {
				// Clear test data
				err := service.ClearData()
				if err != nil {
					t.Errorf("service clear data error %s", err)
				}

				t.Log("successful synchronized to current")

				break out
			}
		}
	}

	service.Stop()
}
