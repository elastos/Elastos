package msg

import (
	"bytes"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type BlocksReq struct {
	Count        uint32
	BlockLocator []*Uint256
	HashStop     Uint256
}

func (msg *BlocksReq) CMD() string {
	return "getblocks"
}

func (msg *BlocksReq) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := WriteElements(buf, msg.Count, msg.BlockLocator, msg.HashStop)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *BlocksReq) Deserialize(body []byte) error {
	var err error
	buf := bytes.NewReader(body)
	msg.Count, err = ReadUint32(buf)
	if err != nil {
		return err
	}

	locator := make([]*Uint256, msg.Count)
	for i := uint32(0); i < msg.Count; i++ {
		var hash Uint256
		err := hash.Deserialize(buf)
		if err != nil {
			return err
		}

		locator = append(locator, &hash)
	}

	err = msg.HashStop.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
