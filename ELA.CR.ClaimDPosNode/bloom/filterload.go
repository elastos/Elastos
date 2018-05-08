package bloom

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type FilterLoad struct {
	Filter    []byte
	HashFuncs uint32
	Tweak     uint32
}

func (msg *FilterLoad) CMD() string {
	return "filterload"
}

func (msg *FilterLoad) Serialize(writer io.Writer) error {
	return common.WriteElements(writer, msg.Filter, msg.HashFuncs, msg.Tweak)
}

func (msg *FilterLoad) Deserialize(reader io.Reader) error {
	return common.ReadElements(reader, &msg.Filter, &msg.HashFuncs, &msg.Tweak)
}
