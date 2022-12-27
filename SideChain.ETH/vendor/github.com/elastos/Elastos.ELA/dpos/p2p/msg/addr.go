package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

const (
	// maxHostLength defines the maximum host name length.
	maxHostLength = 253
)

// Ensure Ping implement p2p.Message interface.
var _ p2p.Message = (*Addr)(nil)

// Addr represents a DPoS network address for connect to a peer.
type Addr struct {
	Host string
	Port uint16
}

func NewAddr(host string, port uint16) *Addr {
	return &Addr{Host: host, Port: port}
}

func (msg *Addr) CMD() string {
	return p2p.CmdAddr
}

func (msg *Addr) MaxLength() uint32 {
	return maxHostLength + 2
}

func (msg *Addr) Serialize(w io.Writer) error {
	if err := common.WriteVarString(w, msg.Host); err != nil {
		return err
	}

	return common.WriteElement(w, msg.Port)
}

func (msg *Addr) Deserialize(r io.Reader) error {
	var err error
	if msg.Host, err = common.ReadVarString(r); err != nil {
		return err
	}

	msg.Port, err = common.ReadUint16(r)
	return err
}
