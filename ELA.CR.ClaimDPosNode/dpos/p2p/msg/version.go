package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	Version  uint32
	Services uint64
	PID      common.Uint256
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 44
}

func (msg *Version) Serialize(w io.Writer) error {
	return common.WriteElements(w, msg.Version, msg.Services, &msg.PID)
}

func (msg *Version) Deserialize(r io.Reader) error {
	return common.ReadElements(r, &msg.Version, &msg.Services, &msg.PID)
}

func NewVersion(pver uint32, services uint64, pid common.Uint256) *Version {
	return &Version{
		Version:  pver,
		Services: services,
		PID:      pid,
	}
}
