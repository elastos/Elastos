package msg

import (
	"bytes"
	"SPVWallet/core"
)

type NotFound struct {
	Hash core.Uint256
}

func (nf *NotFound) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	_, err := nf.Hash.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (nf *NotFound) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := nf.Hash.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
