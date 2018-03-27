package spv

import (
	tx "Elastos.ELA.SideChain/core/transaction"
	"Elastos.ELA.SideChain/core/transaction/payload"
	spvtx "SPVWallet/core/transaction"
	spvdb "SPVWallet/db"
	"SPVWallet/interface"
	"bytes"
	"errors"
	"math/rand"
)

var spvService *_interface.SPVServiceImpl

func SpvInit() error {
	spvService, err := _interface.NewSPVService(uint64(rand.Int63()))
	if err != nil {
		return errors.New("[Error] " + err.Error())
	}
	spvService.Start()
	return nil
}

func VerifyTransaction(txn *tx.Transaction) error {
	proof := new(spvdb.Proof)

	switch object := txn.Payload.(type) {
	case *payload.IssueToken:
		proofBuf := new(bytes.Buffer)
		if err := object.Proof.Serialize(proofBuf); err != nil {
			return errors.New("IssueToken payload serialize failed")
		}
		if err := proof.Deserialize(proofBuf); err != nil {
			return errors.New("IssueToken payload deserialize failed")
		}
	default:
		return errors.New("Invalid payload")
	}

	txBuf := new(bytes.Buffer)
	txn.Serialize(txBuf)

	spvtxn := new(spvtx.Transaction)
	spvtxn.Deserialize(txBuf)

	if err := spvService.VerifyTransaction(*proof, *spvtxn); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}
