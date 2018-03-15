package message

import (
	"Elastos.ELA/common"
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	. "Elastos.ELA/net/protocol"
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
)

type notFound struct {
	Header
	hash common.Uint256
}

func NewNotFound(hash common.Uint256) ([]byte, error) {
	log.Debug()
	var msg notFound
	msg.hash = hash
	msg.Magic = config.Parameters.Magic
	cmd := "notfound"
	copy(msg.Header.CMD[0:len(cmd)], cmd)
	tmpBuffer := bytes.NewBuffer([]byte{})
	msg.hash.Serialize(tmpBuffer)
	p := new(bytes.Buffer)
	err := binary.Write(p, binary.LittleEndian, tmpBuffer.Bytes())
	if err != nil {
		log.Error("Binary Write failed at new notfound Msg")
		return nil, err
	}
	s := sha256.Sum256(p.Bytes())
	s2 := s[:]
	s = sha256.Sum256(s2)
	buf := bytes.NewBuffer(s[:4])
	binary.Read(buf, binary.LittleEndian, &(msg.Checksum))
	msg.Header.Length = uint32(len(p.Bytes()))
	log.Debug("The message payload length is ", msg.Length)

	m, err := msg.Serialization()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}

	return m, nil
}

func (msg notFound) Serialization() ([]byte, error) {
	hdrBuf, err := msg.Header.Serialization()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	msg.hash.Serialize(buf)

	return buf.Bytes(), err
}

func (msg *notFound) Deserialization(p []byte) error {
	buf := bytes.NewBuffer(p)

	err := binary.Read(buf, binary.LittleEndian, &(msg.Header))
	if err != nil {
		log.Warn("Parse notfound message hdr error")
		return errors.New("Parse notfound message hdr error")
	}

	err = msg.hash.Deserialize(buf)
	if err != nil {
		log.Warn("Parse notfound message error")
		return errors.New("Parse notfound message error")
	}

	return err
}

func (msg notFound) Handle(node Noder) error {
	log.Debug("RX notfound message, hash is ", msg.hash)
	return nil
}
