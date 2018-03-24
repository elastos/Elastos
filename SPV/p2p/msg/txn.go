package msg

import (
	"bytes"
	tx "SPVWallet/core/transaction"
)

type Txn struct {
	tx.Transaction
}

func NewTxn(tx tx.Transaction) *Txn {
	return &Txn{Transaction: tx}
}

func (txn *Txn) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := txn.Transaction.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return BuildMessage("tx", buf.Bytes())
}

func (txn *Txn) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := txn.Transaction.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
