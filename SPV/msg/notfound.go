package msg

import (
	"bytes"
	
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type NotFound struct {
	Hash Uint256
}

func (msg *NotFound) CMD() string {
	return "notfound"
}

func (msg *NotFound) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := msg.Hash.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *NotFound) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := msg.Hash.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
