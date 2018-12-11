package msg

import (
	"io"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	Version   uint32
	Services  uint64
	TimeStamp uint32
	Port      uint16
	Nonce     uint64
	Height    uint64
	Relay     bool
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 35
}

func (msg *Version) Serialize(w io.Writer) error {
	return common.WriteElements(w, msg.Version, msg.Services,
		msg.TimeStamp, msg.Port, msg.Nonce, msg.Height, msg.Relay)
}

func (msg *Version) Deserialize(r io.Reader) error {
	return common.ReadElements(r, &msg.Version, &msg.Services,
		&msg.TimeStamp, &msg.Port, &msg.Nonce, &msg.Height, &msg.Relay)
}

func NewVersion(pver uint32, services, nonce, height uint64,
	disableRelayTx bool) *Version {

	return &Version{
		Version:   pver,
		Services:  services,
		TimeStamp: uint32(time.Now().Unix()),
		Nonce:     nonce,
		Height:    height,
		Relay:     !disableRelayTx,
	}
}
