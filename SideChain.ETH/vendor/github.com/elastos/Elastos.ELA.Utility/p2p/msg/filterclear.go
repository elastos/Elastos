package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure FilterClear implement p2p.Message interface.
var _ p2p.Message = (*FilterClear)(nil)

type FilterClear struct{ empty }

func (msg *FilterClear) CMD() string {
	return p2p.CmdFilterClear
}
