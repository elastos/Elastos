package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// MaxBlockLocatorsPerMsg is the maximum number of block locator hashes allowed
// per message.
const MaxBlockLocatorsPerMsg = 500

// Ensure GetBlocks implement p2p.Message interface.
var _ p2p.Message = (*GetBlocks)(nil)

type GetBlocks struct {
	Locator  []*common.Uint256
	HashStop common.Uint256
}

func NewGetBlocks(locator []*common.Uint256, hashStop common.Uint256) *GetBlocks {
	msg := new(GetBlocks)
	msg.Locator = locator
	msg.HashStop = hashStop
	return msg
}

func (msg *GetBlocks) CMD() string {
	return p2p.CmdGetBlocks
}

func (msg *GetBlocks) MaxLength() uint32 {
	return 4 + (MaxBlockLocatorsPerMsg * common.UINT256SIZE) + common.UINT256SIZE
}

func (msg *GetBlocks) Serialize(w io.Writer) error {
	count := len(msg.Locator)
	if count > MaxBlockLocatorsPerMsg {
		str := fmt.Sprintf("too many block locator hashes for message "+
			"[count %v, max %v]", count, MaxBlockLocatorsPerMsg)
		return common.FuncError("GetBlocks.Serialize", str)
	}

	err := common.WriteUint32(w, uint32(count))
	if err != nil {
		return err
	}

	for _, hash := range msg.Locator {
		if err := hash.Serialize(w); err != nil {
			return err
		}
	}

	return msg.HashStop.Serialize(w)
}

func (msg *GetBlocks) Deserialize(reader io.Reader) error {
	count, err := common.ReadUint32(reader)
	if err != nil {
		return err
	}
	if count > MaxBlockLocatorsPerMsg {
		str := fmt.Sprintf("too many block locator hashes for message "+
			"[count %v, max %v]", count, MaxBlockLocatorsPerMsg)
		return common.FuncError("GetBlocks.Deserialize", str)
	}

	// Create a contiguous slice of hashes to deserialize into in order to
	// reduce the number of allocations.
	locator := make([]common.Uint256, count)
	msg.Locator = make([]*common.Uint256, 0, count)
	for i := uint32(0); i < count; i++ {
		hash := &locator[i]
		if err := hash.Deserialize(reader); err != nil {
			return err
		}
		msg.Locator = append(msg.Locator, hash)
	}

	return msg.HashStop.Deserialize(reader)
}
