package msg

import (
	"bytes"
	"SPVWallet/bloom"
	"SPVWallet/core/serialization"
)

type FilterLoad struct {
	Filter    []byte
	HashFuncs uint32
	Tweak     uint32
}

func NewFilterLoad(filter *bloom.Filter) *FilterLoad {
	filterLoad := new(FilterLoad)
	filterLoad.Filter = filter.Filter
	filterLoad.HashFuncs = filter.HashFuncs
	filterLoad.Tweak = filter.Tweak
	return filterLoad
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

	return BuildMessage("filterload", buf.Bytes())
}

func (fl *FilterLoad) Deserialize(msg []byte) error {
	var err error
	buf := bytes.NewReader(msg)
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
