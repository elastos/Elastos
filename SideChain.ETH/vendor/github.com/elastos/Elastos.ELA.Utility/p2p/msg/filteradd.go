package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// MaxFilterAddDataSize is the maximum byte size of a data
	// element to add to the Bloom filter.  It is equal to the
	// maximum element size of a script.
	MaxFilterAddDataSize = 520
)

// Ensure FilterAdd implement p2p.Message interface.
var _ p2p.Message = (*FilterAdd)(nil)

type FilterAdd struct {
	Data []byte
}

func (msg *FilterAdd) CMD() string {
	return p2p.CmdFilterAdd
}

func (msg *FilterAdd) MaxLength() uint32 {
	return 3 + MaxFilterAddDataSize
}

func (msg *FilterAdd) Serialize(writer io.Writer) error {
	size := len(msg.Data)
	if size > MaxFilterAddDataSize {
		str := fmt.Sprintf("filteradd size too large for message "+
			"[size %v, max %v]", size, MaxFilterAddDataSize)
		return common.FuncError("FilterAdd.Serialize", str)
	}

	return common.WriteVarBytes(writer, msg.Data)
}

func (msg *FilterAdd) Deserialize(reader io.Reader) error {
	var err error
	msg.Data, err = common.ReadVarBytes(reader, MaxFilterAddDataSize,
		"filteradd data")
	return err
}
