package msg

import (
	"io"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	Version   uint32
	Services  uint64
	Timestamp time.Time
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
	var timestamp = uint32(msg.Timestamp.Unix())
	return common.WriteElements(w, msg.Version, msg.Services, timestamp,
		msg.Port, msg.Nonce, msg.Height, msg.Relay)
}

func (msg *Version) Deserialize(r io.Reader) error {
	var timestamp uint32
	err := common.ReadElements(r, &msg.Version, &msg.Services, &timestamp,
		&msg.Port, &msg.Nonce, &msg.Height, &msg.Relay)
	if err != nil {
		return err
	}

	msg.Timestamp = time.Unix(int64(timestamp), 0)
	return nil
}

func NewVersion(pver uint32, port uint16, services, nonce, height uint64,
	disableRelayTx bool) *Version {
	return &Version{
		Version:   pver,
		Services:  services,
		Timestamp: time.Unix(time.Now().Unix(), 0),
		Port:      port,
		Nonce:     nonce,
		Height:    height,
		Relay:     !disableRelayTx,
	}
}
