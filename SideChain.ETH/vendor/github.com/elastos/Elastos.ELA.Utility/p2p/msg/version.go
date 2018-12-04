package msg

import (
	"io"
	"encoding/binary"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type Version struct {
	Version   uint32
	Services  uint64
	TimeStamp uint32
	Port      uint16
	Nonce     uint64
	Height    uint64
	Relay     uint8
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 35
}

func (msg *Version) Serialize(writer io.Writer) error {
	return binary.Write(writer, binary.LittleEndian, msg)
}

func (msg *Version) Deserialize(reader io.Reader) error {
	return binary.Read(reader, binary.LittleEndian, msg)
}
