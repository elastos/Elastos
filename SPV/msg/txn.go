package msg

import (
	"bytes"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
)

type Txn struct {
	tx.Transaction
}

func NewTxn(tx tx.Transaction) *Txn {
	return &Txn{Transaction: tx}
}

func (msg *Txn) CMD() string {
	return "tx"
}

func (msg *Txn) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := msg.Transaction.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *Txn) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := msg.Transaction.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
