// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
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
	Flags     uint8
}

func (msg *FilterLoad) CMD() string {
	return p2p.CmdFilterLoad
}

func (msg *FilterLoad) MaxLength() uint32 {
	return 3 + MaxFilterLoadFilterSize + 8 + 1
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

	return common.WriteElements(w, msg.HashFuncs, msg.Tweak, msg.Flags)
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

	// deserialize flags ignore the result for compatibility with previous versions
	common.ReadElements(r, &msg.Flags)

	return nil
}
