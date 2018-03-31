package p2p

import (
	"bytes"
	"encoding/binary"
	"github.com/elastos/Elastos.ELA.SPV/common/serialization"
)

type Addrs struct {
	Count uint64
	Addrs []Addr
}

func NewAddrs(addrs []Addr) *Addrs {
	msg := new(Addrs)
	msg.Count = uint64(len(addrs))
	msg.Addrs = addrs
	return msg
}

func (msg *Addrs) CMD() string {
	return "addr"
}

func (msg *Addrs) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteElements(buf, msg.Count, msg.Addrs)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *Addrs) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := binary.Read(buf, binary.LittleEndian, &msg.Count)
	if err != nil {
		return err
	}

	msg.Addrs = make([]Addr, msg.Count)
	err = binary.Read(buf, binary.LittleEndian, &msg.Addrs)
	if err != nil {
		return err
	}

	return nil
}
