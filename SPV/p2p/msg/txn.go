package msg

import (
	"bytes"
	"encoding/binary"

	tx "SPVWallet/core/transaction"
)

type Txn struct {
	Header
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
	err := binary.Read(buf, binary.LittleEndian, &txn.Header)
	if err != nil {
		return err
	}

	err = txn.Transaction.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
