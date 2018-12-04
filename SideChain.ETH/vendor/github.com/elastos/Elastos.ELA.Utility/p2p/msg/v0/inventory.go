package v0

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const MaxInvPerMsg = 100

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
	return common.WriteElements(w, uint32(len(msg.Hashes)), msg.Hashes)
}

func (msg *Inv) Deserialize(r io.Reader) error {
	count, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	// Limit to max inventory vectors per message.
	if count > MaxInvPerMsg {
		return fmt.Errorf("too many invvect in message [%v]", count)
	}

	msg.Hashes = make([]*common.Uint256, 0, count)
	for i := uint32(0); i < count; i++ {
		var hash common.Uint256
		if err := hash.Deserialize(r); err != nil {
			return err
		}
		msg.Hashes = append(msg.Hashes, &hash)
	}

	return nil
}
