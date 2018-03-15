package message

import (
	"bytes"
	"encoding/binary"

	"Elastos.ELA/common/serialization"
	"Elastos.ELA/net/protocol"
	"Elastos.ELA/bloom"
)

type FilterLoad struct {
	Header
	Filter    []byte
	HashFuncs uint32
	Tweak     uint32
}

func (fl *FilterLoad) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, fl.Header)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteVarBytes(buf, fl.Filter)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteUint32(buf, fl.HashFuncs)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteUint32(buf, fl.Tweak)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (fl *FilterLoad) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &fl.Header)
	if err != nil {
		return err
	}

	fl.Filter, err = serialization.ReadVarBytes(buf)
	if err != nil {
		return err
	}

	fl.HashFuncs, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	fl.Tweak, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	return nil
}

func (fl *FilterLoad) Handle(node protocol.Noder) error {
	node.LoadFilter(bloom.LoadFilter(fl.Filter, fl.HashFuncs, fl.Tweak))
	return nil
}
