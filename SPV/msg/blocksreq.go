package msg

import (
	"bytes"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/common/serialization"
)

type BlocksReq struct {
	Count        uint32
	BlockLocator []*Uint256
	HashStop     Uint256
}

func NewBlocksReq(locator []*Uint256, hashStop Uint256) *BlocksReq {
	blocksReq := new(BlocksReq)
	blocksReq.Count = uint32(len(locator))
	blocksReq.BlockLocator = locator
	blocksReq.HashStop = hashStop
	return blocksReq
}

func (msg *BlocksReq) CMD() string {
	return "getblocks"
}

func (msg *BlocksReq) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteElements(buf, msg.Count, msg.BlockLocator, msg.HashStop)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *BlocksReq) Deserialize(body []byte) error {
	var err error
	buf := bytes.NewReader(body)
	msg.Count, err = serialization.ReadUint32(buf)
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
