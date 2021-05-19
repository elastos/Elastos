package msg

import (
	"io"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

// Ensure Nep5LogMsgRes implement p2p.Message interface.
var _ p2p.Message = (*Nep5LogMsgRes)(nil)

type Nep5LogMsgRes struct {
	Logs      []types.Nep5Log
}

func (msg *Nep5LogMsgRes) MaxLength() uint32 {
	return math.MaxUint32
}

func (msg *Nep5LogMsgRes) CMD() string {
	return CmdNep5Log
}

func (msg *Nep5LogMsgRes) Serialize(w io.Writer) error {
	len := len(msg.Logs)
	common.WriteUint32(w, uint32(len))
	for i := 0; i < len; i++ {
		err := msg.Logs[i].Serialize(w, 0)
		if err != nil {
			return err
		}
	}
	return nil
}

func (msg *Nep5LogMsgRes) Deserialize(r io.Reader) error {
	len, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	msg.Logs = make([]types.Nep5Log, len)
	for i := 0; i < int(len); i++ {
		err := msg.Logs[i].Deserialize(r, 0)
		if err != nil {
			return err
		}
	}
	return nil
}
