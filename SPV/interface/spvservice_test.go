package _interface

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"fmt"
	"testing"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
	. "github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/core"
)

var spv SPVService

type TxListener struct {
	address string
	txType  TransactionType
	flags   uint64
}

func (l *TxListener) Address() string {
	return l.address
}

func (l *TxListener) Type() TransactionType {
	return l.txType
}

func (l *TxListener) Flags() uint64 {
	return l.flags
}

func (l *TxListener) Notify(id common.Uint256, proof MerkleProof, tx Transaction) {
	fmt.Printf("Receive notify ID: %s, Type: %s\n", id.String(), tx.TxType.Name())
	err := spv.VerifyTransaction(proof, tx)
	if err != nil {
		fmt.Println("Verify transaction error:", err)
	}
	// Submit transaction receipt
	spv.SubmitTransactionReceipt(id, tx.Hash())
}

func (l *TxListener) Rollback(height uint32) {}

func TestGetListenerKey(t *testing.T) {
	var key1, key2 common.Uint256
	listener := &TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	}

	key1 = getListenerKey(listener)
	key2 = getListenerKey(&TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  CoinBase,
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
	listener.txType = TransferAsset
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
	log.Init(0, 5, 20)

	var id = make([]byte, 8)
	var clientId uint64
	var err error
	rand.Read(id)
	binary.Read(bytes.NewReader(id), binary.LittleEndian, clientId)
	spv, err = NewSPVService(config.Values().Magic, config.Values().Foundation, clientId, config.Values().SeedList, 8, 100)
	if err != nil {
		t.Error("NewSPVService error %s", err.Error())
	}

	confirmedListener := &TxListener{
		address: "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
		txType:  CoinBase,
		flags:   FlagNotifyConfirmed | FlagNotifyInSyncing,
	}

	unconfirmedListener := &TxListener{
		address: "Ef2bDPwcUKguteJutJQCmjX2wgHVfkJ2Wq",
		txType:  TransferAsset,
		flags:   0,
	}

	// Set on transaction confirmed callback
	spv.RegisterTransactionListener(confirmedListener)
	spv.RegisterTransactionListener(unconfirmedListener)

	// Start spv service
	err = spv.Start()
	if err != nil {
		t.Error("Start SPV service error: ", err)
	}
}
