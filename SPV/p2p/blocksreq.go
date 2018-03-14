package p2p

import (
	"bytes"
	. "SPVWallet/core"
	"SPVWallet/core/serialization"
	"encoding/binary"
)

type BlocksReq struct {
	Header
	Count        uint32
	BlockLocator []*Uint256
	HashStop     Uint256
}

func NewBlocksReqMsg(locator []*Uint256, hashStop Uint256) ([]byte, error) {
	msg := new(BlocksReq)
	msg.Count = uint32(len(locator))
	msg.BlockLocator = locator
	msg.HashStop = hashStop

	buf := new(bytes.Buffer)
	err := serialization.WriteUint32(buf, msg.Count)
	if err != nil {
		return nil, err
	}

	for _, hash := range msg.BlockLocator {
		_, err := hash.Serialize(buf)
		if err != nil {
			return nil, err
		}
	}

	_, err = msg.HashStop.Serialize(buf)
	if err != nil {
		return nil, err
	}

	msg.Header = *BuildHeader("getblocks", buf.Bytes())

	return msg.Serialize()
}

func (br *BlocksReq) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, br.Header)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteUint32(buf, br.Count)
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

	return buf.Bytes(), nil
}

func (br *BlocksReq) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &br.Header)
	if err != nil {
		return err
	}

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
