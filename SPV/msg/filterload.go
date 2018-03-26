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

func (msg *FilterLoad) CMD() string {
	return "filterload"
}

func (msg *FilterLoad) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteElements(buf, msg.Filter, msg.HashFuncs, msg.Tweak)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *FilterLoad) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := serialization.ReadElements(buf, &msg.Filter, &msg.HashFuncs, &msg.Tweak)
	if err != nil {
		return err
	}

	return nil
}
