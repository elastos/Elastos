package msg

import (
	"bytes"
	tx "SPVWallet/core/transaction"
)

type Txn struct {
	tx.Transaction
}

func NewTxnMsg(txn tx.Transaction) ([]byte, error) {
	msg := new(Txn)
	msg.Transaction = txn

	body, err := msg.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("tx", body)
}

func (txn *Txn) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := txn.Transaction.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (txn *Txn) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := txn.Transaction.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
