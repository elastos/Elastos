package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const defaultInvListSize = 100

// Ensure Inv implement p2p.Message interface.
var _ p2p.Message = (*Inv)(nil)

type Inv struct {
	InvList []*InvVect
}

func NewInv() *Inv {
	msg := &Inv{
		InvList: make([]*InvVect, 0, defaultInvListSize),
	}
	return msg
}

func NewInvSize(size uint) *Inv {
	// Limit the specified hint to the maximum allow per message.
	if size > MaxInvPerMsg {
		size = MaxInvPerMsg
	}

	return &Inv{
		InvList: make([]*InvVect, 0, size),
	}
}

// AddInvVect adds an inventory vector to the message.
func (msg *Inv) AddInvVect(iv *InvVect) error {
	if len(msg.InvList)+1 > MaxInvPerMsg {
		return fmt.Errorf("AddInvVect too many invvect in message [max %v]", MaxInvPerMsg)
	}

	msg.InvList = append(msg.InvList, iv)
	return nil
}

func (msg *Inv) CMD() string {
	return p2p.CmdInv
}

func (msg *Inv) MaxLength() uint32 {
	return 4 + (MaxInvPerMsg * maxInvVectPayload)
}

func (msg *Inv) Serialize(w io.Writer) error {
	// Limit to max inventory vectors per message.
	count := len(msg.InvList)
	if count > MaxInvPerMsg {
		str := fmt.Sprintf("too many invvect in message [%v]", count)
		return common.FuncError("Inv.Serialize", str)
	}

	err := common.WriteUint32(w, uint32(len(msg.InvList)))
	if err != nil {
		return err
	}

	for _, iv := range msg.InvList {
		if err := iv.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (msg *Inv) Deserialize(r io.Reader) error {
	count, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	// Limit to max inventory vectors per message.
	if count > MaxInvPerMsg {
		str := fmt.Sprintf("too many invvect in message [%v]", count)
		return common.FuncError("Inv.Deserialize", str)
	}

	// Create a contiguous slice of inventory vectors to deserialize into in
	// order to reduce the number of allocations.
	invList := make([]InvVect, count)
	msg.InvList = make([]*InvVect, 0, count)
	for i := uint32(0); i < count; i++ {
		iv := &invList[i]
		err := iv.Deserialize(r)
		if err != nil {
			return err
		}
		msg.InvList = append(msg.InvList, iv)
	}

	return nil
}
