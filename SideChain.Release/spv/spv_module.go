package spv

import (
	tx "Elastos.ELA.SideChain/core/transaction"
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

func VerifyTransaction(spvInfo []byte, txn *tx.Transaction) error {
	buf := new(bytes.Buffer)
	txn.Serialize(buf)
	txBytes := buf.Bytes()

	r := bytes.NewReader(txBytes)
	spvtxn := new(spvtx.Transaction)
	spvtxn.Deserialize(r)

	proof := new(spvdb.Proof)
	proof.Deserialize(spvInfo)
	if err := spvService.VerifyTransaction(*proof, *spvtxn); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}
