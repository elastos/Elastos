package p2p

import (
	"bytes"
	"encoding/binary"
)

type Addrs struct {
	Header
	Count     uint64
	PeerAddrs []PeerAddr
}

func NewAddrsMsg(addrs []PeerAddr) ([]byte, error) {
	msg := new(Addrs)
	msg.Count = uint64(len(addrs))
	msg.PeerAddrs = addrs

	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, msg.Count)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, msg.PeerAddrs)
	if err != nil {
		return nil, err
	}

	msg.Header = *BuildHeader("addr", buf.Bytes())

	return msg.Serialize()
}

func (addrs *Addrs) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, addrs.Header)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, addrs.Count)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, addrs.PeerAddrs)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (addrs *Addrs) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &addrs.Header)
	if err != nil {
		return err
	}

	err = binary.Read(buf, binary.LittleEndian, &addrs.Count)
	if err != nil {
		return err
	}

	addrs.PeerAddrs = make([]PeerAddr, addrs.Count)
	err = binary.Read(buf, binary.LittleEndian, &addrs.PeerAddrs)
	if err != nil {
		return err
	}

	return nil
}
