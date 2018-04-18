package msg

import (
	"bytes"
	. "github.com/elastos/Elastos.ELA.Utility/core"
)

type Txn struct {
	Transaction
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
