package msg

import (
	"bytes"
	. "SPVWallet/core"
	"SPVWallet/core/serialization"
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

func (br *BlocksReq) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint32(buf, br.Count)
	if err != nil {
		return nil, err
	}

	for _, hash := range br.BlockLocator {
		_, err := hash.Serialize(buf)
		if err != nil {
			return nil, err
		}
	}

	_, err = br.HashStop.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return BuildMessage("getblocks", buf.Bytes())
}

func (br *BlocksReq) Deserialize(msg []byte) error {
	var err error
	buf := bytes.NewReader(msg)
	br.Count, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	locator := make([]*Uint256, br.Count)
	for i := uint32(0); i < br.Count; i++ {
		var hash Uint256
		err := hash.Deserialize(buf)
		if err != nil {
			return err
		}

		locator = append(locator, &hash)
	}

	err = br.HashStop.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
