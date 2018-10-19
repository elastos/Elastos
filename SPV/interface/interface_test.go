package _interface

import (
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/sync"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/http/jsonrpc"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
	"github.com/elastos/Elastos.ELA/core"
)

var service SPVService

type TxListener struct {
	address string
	txType  core.TransactionType
	flags   uint64
}

func (l *TxListener) Address() string {
	return l.address
}

func (l *TxListener) Type() core.TransactionType {
	return l.txType
}

func (l *TxListener) Flags() uint64 {
	return l.flags
}

func (l *TxListener) Notify(id common.Uint256, proof bloom.MerkleProof, tx core.Transaction) {
	fmt.Printf("Receive notify ID: %s, Type: %s\n", id.String(), tx.TxType.Name())
	err := service.VerifyTransaction(proof, tx)
	if err != nil {
		fmt.Println("Verify transaction error:", err)
	}
	// Submit transaction receipt
	service.SubmitTransactionReceipt(id, tx.Hash())
}

func (l *TxListener) Rollback(height uint32) {}

func TestGetListenerKey(t *testing.T) {
	var key1, key2 common.Uint256
	listener := &TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  core.CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	}

	key1 = getListenerKey(listener)
	key2 = getListenerKey(&TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  core.CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	})
	if !key1.IsEqual(key2) {
		t.Errorf("listeners with same fields get different key1 %s, key2 %s", key1.String(), key2.String())
	}
	t.Log("listeners with same fields passed")

	// same type, flags different address
	key1 = getListenerKey(listener)
	listener.address = "Ef2bDPwcUKguteJutJQCmjX2wgHVfkJ2Wq"
	key2 = getListenerKey(listener)
	if key1.IsEqual(key2) {
		t.Errorf("listeners with different address got same key %s", key1.String())
	}
	t.Log("listeners with different address passed")

	// same address, flags different type
	key1 = getListenerKey(listener)
	listener.txType = core.TransferAsset
	key2 = getListenerKey(listener)
	if key1.IsEqual(key2) {
		t.Errorf("listeners with different type got same key %s", key1.String())
	}
	t.Log("listeners with different type passed")

	// same address, type different flags
	key1 = getListenerKey(listener)
	listener.flags = FlagNotifyInSyncing
	key2 = getListenerKey(listener)
	key2 = getListenerKey(listener)
	if key1.IsEqual(key2) {
		t.Errorf("listeners with different flags got same key %s", key1.String())
	}
	t.Log("listeners with different flags passed")
}

func TestNewSPVService(t *testing.T) {
	backend := elalog.NewBackend(os.Stdout, elalog.Lshortfile)
	admrlog := backend.Logger("ADMR", elalog.LevelOff)
	cmgrlog := backend.Logger("CMGR", elalog.LevelOff)
	bcdblog := backend.Logger("BCDB", elalog.LevelTrace)
	synclog := backend.Logger("SYNC", elalog.LevelTrace)
	peerlog := backend.Logger("PEER", elalog.LevelTrace)
	spvslog := backend.Logger("SPVS", elalog.LevelTrace)
	srvrlog := backend.Logger("SRVR", elalog.LevelTrace)
	rpcslog := backend.Logger("RPCS", elalog.LevelTrace)

	addrmgr.UseLogger(admrlog)
	connmgr.UseLogger(cmgrlog)
	blockchain.UseLogger(bcdblog)
	sdk.UseLogger(spvslog)
	jsonrpc.UseLogger(rpcslog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	store.UseLogger(bcdblog)
	sync.UseLogger(synclog)

	seedList := []string{
		"node-regtest-201.elastos.org:22866",
		"node-regtest-202.elastos.org:22866",
		"node-regtest-203.elastos.org:22866",
		"node-regtest-204.elastos.org:22866",
		"node-regtest-205.elastos.org:22866",
		"node-regtest-206.elastos.org:22866",
		"node-regtest-207.elastos.org:22866",
	}

	config := &Config{
		Magic:          20180627,
		Foundation:     "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",
		SeedList:       seedList,
		DefaultPort:    22866,
		MinOutbound:    8,
		MaxConnections: 100,
	}

	service, err := newSpvService(config)
	if err != nil {
		t.Error("NewSPVService error %s", err.Error())
	}

	confirmedListener := &TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  core.CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	}

	unconfirmedListener := &TxListener{
		address: "Ef2bDPwcUKguteJutJQCmjX2wgHVfkJ2Wq",
		txType:  core.TransferAsset,
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
		case <-syncTicker.C:

			if service.IService.IsCurrent() {
				// Clear test data
				err := service.ClearData()
				if err != nil {
					t.Errorf("service clear data error %s", err)
				}

				service.Stop()
				t.Log("successful synchronized to current")

				break out
			}
		}
	}
}
