package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type FilterClear struct {
}

func (msg *FilterClear) CMD() string {
	return p2p.CmdFilterClear
}

func (msg *FilterClear) MaxLength() uint32 {
	return 0
}

func (msg *FilterClear) Serialize(writer io.Writer) error {
	return nil
}

func (msg *FilterClear) Deserialize(reader io.Reader) error {
	return nil
}
