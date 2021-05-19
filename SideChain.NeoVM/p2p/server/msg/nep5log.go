package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/common"

)

// Ensure Nep5LogMsg implement p2p.Message interface.
var _ p2p.Message = (*Nep5LogMsg)(nil)

type Nep5LogMsg struct {
	blockHash common.Uint256
	height uint32
	topicAddr common.Uint160
}

func (msg *Nep5LogMsg) GetBlockHash() common.Uint256 {
	return msg.blockHash
}

func (msg *Nep5LogMsg) GetHeight() uint32 {
	return msg.height
}

func (msg *Nep5LogMsg) GetTopicAddr() common.Uint160 {
	return msg.topicAddr
}

func (msg *Nep5LogMsg) MaxLength() uint32 {
	return 4 + 32 + 4 + 20
}


func (msg *Nep5LogMsg) CMD() string {
	return CmdNep5Log
}


func (msg *Nep5LogMsg) Serialize(w io.Writer) error {
	err := msg.blockHash.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteUint32(w, msg.height)
	if err != nil {
		return err
	}

	err = msg.topicAddr.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

func (msg *Nep5LogMsg) Deserialize(r io.Reader) error {
	err := msg.blockHash.Deserialize(r)
	if err != nil {
		return err
	}

	msg.height, err = common.ReadUint32(r)
	if err != nil {
		return err
	}

	err = msg.topicAddr.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}
