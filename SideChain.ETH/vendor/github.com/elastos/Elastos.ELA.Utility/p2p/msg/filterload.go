package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"fmt"
)

const (
	// MaxFilterLoadHashFuncs is the maximum number of hash functions to
	// load into the Bloom filter.
	MaxFilterLoadHashFuncs = 50

	// MaxFilterLoadFilterSize is the maximum size in bytes a filter may be.
	MaxFilterLoadFilterSize = 36000
)

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

func (msg *FilterLoad) Serialize(writer io.Writer) error {
	if msg.HashFuncs > MaxFilterLoadHashFuncs {
		return fmt.Errorf("too many filter hash functions for message "+
			"[count %v, max %v]", msg.HashFuncs, MaxFilterLoadHashFuncs)
	}

	return common.WriteElements(writer, msg.Filter, msg.HashFuncs, msg.Tweak)
}

func (msg *FilterLoad) Deserialize(reader io.Reader) error {
	err :=  common.ReadElements(reader, &msg.Filter, &msg.HashFuncs, &msg.Tweak)
	if err != nil {
		return err
	}

	if msg.HashFuncs > MaxFilterLoadHashFuncs {
		return fmt.Errorf("too many filter hash functions for message "+
			"[count %v, max %v]", msg.HashFuncs, MaxFilterLoadHashFuncs)
	}

	return nil
}
