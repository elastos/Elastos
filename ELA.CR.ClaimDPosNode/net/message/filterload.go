package message

import (
	"bytes"
	"encoding/binary"

	"Elastos.ELA/bloom"
	"Elastos.ELA/common/serialization"
	"Elastos.ELA/net/protocol"
	"Elastos.ELA/common/log"
)

type FilterLoad struct {
	Header
	Filter    []byte
	HashFuncs uint32
	Tweak     uint32
}

func NewFilterLoadMsg(filter *bloom.Filter) ([]byte, error) {
	msg := new(FilterLoad)
	msg.Filter = filter.Filter
	msg.HashFuncs = filter.HashFuncs
	msg.Tweak = filter.Tweak

	body, err := msg.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("filterload", body)
}

func (fl *FilterLoad) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteVarBytes(buf, fl.Filter)
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
	log.Debug(">>>>> FilterLoad message received:", *fl)
	node.LoadFilter(bloom.LoadFilter(fl.Filter, fl.HashFuncs, fl.Tweak))
	return nil
}
