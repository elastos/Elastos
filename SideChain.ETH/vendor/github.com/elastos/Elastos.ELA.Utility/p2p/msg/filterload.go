package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// MaxFilterLoadHashFuncs is the maximum number of hash functions to
	// load into the Bloom filter.
	MaxFilterLoadHashFuncs = 50

	// MaxFilterLoadFilterSize is the maximum size in bytes a filter may be.
	MaxFilterLoadFilterSize = 36000
)

// Ensure FilterLoad implement p2p.Message interface.
var _ p2p.Message = (*FilterLoad)(nil)

type FilterLoad struct {
	Filter    []byte
	HashFuncs uint32
	Tweak     uint32
}

func (msg *FilterLoad) CMD() string {
	return p2p.CmdFilterLoad
}

func (msg *FilterLoad) MaxLength() uint32 {
	return 3 + MaxFilterLoadFilterSize + 8
}

func (msg *FilterLoad) Serialize(w io.Writer) error {
	size := len(msg.Filter)
	if size > MaxFilterLoadFilterSize {
		str := fmt.Sprintf("filterload filter size too large for message "+
			"[size %v, max %v]", size, MaxFilterLoadFilterSize)
		return common.FuncError("MsgFilterLoad.BtcEncode", str)
	}

	if msg.HashFuncs > MaxFilterLoadHashFuncs {
		str := fmt.Sprintf("too many filter hash functions for message "+
			"[count %v, max %v]", msg.HashFuncs, MaxFilterLoadHashFuncs)
		return common.FuncError("MsgFilterLoad.BtcEncode", str)
	}

	err := common.WriteVarBytes(w, msg.Filter)
	if err != nil {
		return err
	}

	return common.WriteElements(w, msg.HashFuncs, msg.Tweak)
}

func (msg *FilterLoad) Deserialize(r io.Reader) error {
	var err error
	msg.Filter, err = common.ReadVarBytes(r, MaxFilterLoadFilterSize,
		"filterload filter size")
	if err != nil {
		return err
	}

	err = common.ReadElements(r, &msg.HashFuncs, &msg.Tweak)
	if err != nil {
		return err
	}

	if msg.HashFuncs > MaxFilterLoadHashFuncs {
		str := fmt.Sprintf("too many filter hash functions for message "+
			"[count %v, max %v]", msg.HashFuncs, MaxFilterLoadHashFuncs)
		return common.FuncError("FilterLoad.Deserialize", str)
	}

	return nil
}
