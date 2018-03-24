package p2p

import (
	"bytes"
	"encoding/binary"
)

type Addrs struct {
	Count     uint64
	PeerAddrs []Addr
}

func NewAddrs(addrs []Addr) *Addrs {
	msg := new(Addrs)
	msg.Count = uint64(len(addrs))
	msg.PeerAddrs = addrs
	return msg
}

func (msg *Addrs) CMD() string {
	return "addr"
}

func (msg *Addrs) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, msg.Count)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, msg.PeerAddrs)
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

	msg.PeerAddrs = make([]Addr, msg.Count)
	err = binary.Read(buf, binary.LittleEndian, &msg.PeerAddrs)
	if err != nil {
		return err
	}

	return nil
}
