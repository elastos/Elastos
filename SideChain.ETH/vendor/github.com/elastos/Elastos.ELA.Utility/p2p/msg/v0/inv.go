package v0

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const MaxInvPerMsg = 100

// Ensure Inv implement p2p.Message interface.
var _ p2p.Message = (*Inv)(nil)

type Inv struct {
	Hashes []*common.Uint256
}

func NewInv(hashes []*common.Uint256) *Inv {
	return &Inv{Hashes: hashes}
}

func (msg *Inv) CMD() string {
	return p2p.CmdInv
}

func (msg *Inv) MaxLength() uint32 {
	return 4 + (MaxInvPerMsg * common.UINT256SIZE)
}

func (msg *Inv) Serialize(w io.Writer) error {
	// Limit to max inventory vectors per message.
	count := len(msg.Hashes)
	if count > MaxInvPerMsg {
		str := fmt.Sprintf("too many invvect in message [%v]", count)
		return common.FuncError("Inv.Serialize", str)
	}

	err := common.WriteUint32(w, uint32(len(msg.Hashes)))
	if err != nil {
		return err
	}

	for _, hash := range msg.Hashes {
		if err := hash.Serialize(w); err != nil {
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
	hashes := make([]common.Uint256, count)
	msg.Hashes = make([]*common.Uint256, 0, count)
	for i := uint32(0); i < count; i++ {
		hash := &hashes[i]
		err := hash.Deserialize(r)
		if err != nil {
			return err
		}
		msg.Hashes = append(msg.Hashes, hash)
	}

	return nil
}