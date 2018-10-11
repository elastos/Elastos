package _interface

import (
	"fmt"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	spvpeer "github.com/elastos/Elastos.ELA.SPV/peer"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
	"github.com/elastos/Elastos.ELA.SPV/sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA.Utility/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/connmgr"
	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
	"github.com/elastos/Elastos.ELA/bloom"
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
	addrmgr.UseLogger(elalog.Stdout)
	connmgr.UseLogger(elalog.Stdout)
	sdk.UseLogger(elalog.Stdout)
	//rpc.UseLogger(logger)
	//peer.UseLogger(elalog.Stdout)
	spvpeer.UseLogger(elalog.Stdout)
	server.UseLogger(elalog.Stdout)
	blockchain.UseLogger(elalog.Stdout)
	sync.UseLogger(elalog.Stdout)
	UseLogger(elalog.Stdout)

	config := &Config{
		Magic:          config.Values().Magic,
		Foundation:     config.Values().Foundation,
		SeedList:       config.Values().SeedList,
		DefaultPort:    config.Values().DefaultPort,
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
