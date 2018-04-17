package message

import (
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/net/protocol"
)

type FilterLoadMsg struct {
	Header
	Filter bloom.FilterLoad
}

func NewFilterLoadMsg(filter *bloom.Filter) ([]byte, error) {
	msg := new(FilterLoadMsg)
	msg.Filter = *filter.GetFilterLoadMsg()
	body, err := msg.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("filterload", body)
}

func (f *FilterLoadMsg) Serialize() ([]byte, error) {
	return f.Filter.Serialize()
}

func (f *FilterLoadMsg) Deserialize(body []byte) error {
	return f.Filter.Deserialize(body)
}

func (f *FilterLoadMsg) Handle(node protocol.Noder) error {
	node.LoadFilter(&f.Filter)
	return nil
}
