package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	// MaxFilterAddDataSize is the maximum byte size of a data
	// element to add to the Bloom filter.  It is equal to the
	// maximum element size of a script.
	MaxFilterAddDataSize = 520
)

type FilterAdd struct {
	Data []byte
}

func (msg *FilterAdd) CMD() string {
	return p2p.CmdFilterAdd
}

func (msg *FilterAdd) MaxLength() uint32 {
	return 9 + MaxFilterAddDataSize
}

func (msg *FilterAdd) Serialize(writer io.Writer) error {
	return common.WriteVarBytes(writer, msg.Data)
}

func (msg *FilterAdd) Deserialize(reader io.Reader) error {
	var err error
	msg.Data, err = common.ReadVarBytes(reader)
	if err != nil {
		return err
	}
	return nil
}
