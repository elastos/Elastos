package message

import (
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/common/serialize"
	"github.com/elastos/Elastos.ELA/core/ledger"
	. "github.com/elastos/Elastos.ELA/net/protocol"
	"bytes"
	"crypto/sha256"
	"encoding/binary"
)

type pong struct {
	Header
	height uint64
}

func NewPongMsg() ([]byte, error) {
	var msg pong
	msg.Header.Magic = config.Parameters.Magic
	copy(msg.Header.CMD[0:7], "pong")
	msg.height = uint64(ledger.DefaultLedger.Store.GetHeight())
	tmpBuffer := bytes.NewBuffer([]byte{})
	serialize.WriteUint64(tmpBuffer, msg.height)
	b := new(bytes.Buffer)
	err := binary.Write(b, binary.LittleEndian, tmpBuffer.Bytes())
	if err != nil {
		log.Error("Binary Write failed at new Msg")
		return nil, err
	}
	s := sha256.Sum256(b.Bytes())
	s2 := s[:]
	s = sha256.Sum256(s2)
	buf := bytes.NewBuffer(s[:4])
	binary.Read(buf, binary.LittleEndian, &(msg.Header.Checksum))
	msg.Header.Length = uint32(len(b.Bytes()))

	m, err := msg.Serialize()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}
	return m, nil
}

func (msg pong) Handle(node Noder) error {
	node.SetHeight(msg.height)
	return nil
}

func (msg pong) Serialize() ([]byte, error) {
	hdrBuf, err := msg.Header.Serialize()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	err = serialize.WriteUint64(buf, msg.height)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), err

}

func (msg *pong) Deserialize(p []byte) error {
	buf := bytes.NewBuffer(p)
	err := binary.Read(buf, binary.LittleEndian, &(msg.Header))
	if err != nil {
		return err
	}

	msg.height, err = serialize.ReadUint64(buf)
	return err
}
