package msg

import (
	"bytes"
	"encoding/binary"

	"SPVWallet/core"
)

type NotFound struct {
	Header
	Hash core.Uint256
}

func (nf *NotFound) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, nf.Header)
	if err != nil {
		return nil, err
	}

	_, err = nf.Hash.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (nf *NotFound) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &nf.Header)
	if err != nil {
		return err
	}

	err = nf.Hash.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
