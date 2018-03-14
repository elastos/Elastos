package p2p

import (
	tx "SPVWallet/core/transaction"
	"bytes"
	"encoding/binary"
)

type Txn struct {
	Header
	tx.Transaction
}

func NewTxnMsg(txn tx.Transaction) ([]byte, error) {
	tx := new(Txn)
	tx.Transaction = txn

	buf := new(bytes.Buffer)
	txn.Serialize(buf)

	tx.Header = *BuildHeader("tx", buf.Bytes())

	return tx.Serialize()
}

func (txn *Txn) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, txn.Header)
	if err != nil {
		return nil, err
	}

	err = txn.Transaction.Serialize(buf)
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
