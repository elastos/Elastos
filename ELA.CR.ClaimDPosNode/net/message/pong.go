package message

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/net/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type pong struct {
	Hdr
	height uint64
}

func NewPongMsg() ([]byte, error) {
	var msg pong
	msg.Hdr.Magic = config.Parameters.Magic
	copy(msg.Hdr.CMD[0:7], "pong")
	msg.height = uint64(chain.DefaultLedger.Store.GetHeight())
	tmpBuffer := bytes.NewBuffer([]byte{})
	common.WriteUint64(tmpBuffer, msg.height)
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
	binary.Read(buf, binary.LittleEndian, &(msg.Hdr.Checksum))
	msg.Hdr.Length = uint32(len(b.Bytes()))

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
	hdrBuf, err := msg.Hdr.Serialize()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	err = common.WriteUint64(buf, msg.height)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), err

}

func (msg *pong) Deserialize(p []byte) error {
	buf := bytes.NewBuffer(p)
	err := binary.Read(buf, binary.LittleEndian, &(msg.Hdr))
	if err != nil {
		return err
	}

	msg.height, err = common.ReadUint64(buf)
	return err
}
