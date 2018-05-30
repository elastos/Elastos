package _interface

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"testing"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"

	. "github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/core"
)

var spv SPVService

func TestNewSPVService(t *testing.T) {
	log.Init(log.LevelDebug)

	var id = make([]byte, 8)
	var clientId uint64
	var err error
	rand.Read(id)
	binary.Read(bytes.NewReader(id), binary.LittleEndian, clientId)
	spv, err = NewSPVService(config.Values().Magic, clientId, config.Values().SeedList, 8, 100)
	if err != nil {
		t.Error("NewSPVService error %s", err.Error())
	}

	// Set on transaction confirmed callback
	spv.RegisterTransactionListener(&ConfirmedListener{txType: CoinBase})
	spv.RegisterTransactionListener(&UnconfirmedListener{txType: TransferAsset})

	// Start spv service
	err = spv.Start()
	if err != nil {
		t.Error("Start SPV service error: ", err)
	}
}

type ConfirmedListener struct {
	txType TransactionType
}

func (l *ConfirmedListener) Address() string {
	return "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z"
}

func (l *ConfirmedListener) Type() TransactionType {
	return l.txType
}

func (l *ConfirmedListener) Flags() uint64 {
	return FlagNotifyConfirmed | FlagNotifyInSyncing
}

func (l *ConfirmedListener) Notify(proof MerkleProof, tx Transaction) {
	log.Debug("Receive confirmed transaction hash:", tx.Hash().String())
	err := spv.VerifyTransaction(proof, tx)
	if err != nil {
		log.Error("Verify transaction error: ", err)
		return
	}

	// Submit transaction receipt
	spv.SubmitTransactionReceipt(tx.Hash())
}

func (l *ConfirmedListener) Rollback(height uint32) {}

type UnconfirmedListener struct {
	txType TransactionType
}

func (l *UnconfirmedListener) Address() string {
	return "Ef2bDPwcUKguteJutJQCmjX2wgHVfkJ2Wq"
}

func (l *UnconfirmedListener) Type() TransactionType {
	return l.txType
}

func (l *UnconfirmedListener) Flags() uint64 {
	return FlagNotifyInSyncing
}

func (l *UnconfirmedListener) Notify(proof MerkleProof, tx Transaction) {
	log.Debug("Receive unconfirmed transaction hash:", tx.Hash().String())
	err := spv.VerifyTransaction(proof, tx)
	if err != nil {
		log.Error("Verify transaction error: ", err)
		return
	}

	// Submit transaction receipt
	spv.SubmitTransactionReceipt(tx.Hash())
}

func (l *UnconfirmedListener) Rollback(height uint32) {}
