// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type RequestConsensus struct {
	Height uint32
}

func (msg *RequestConsensus) CMD() string {
	return CmdRequestConsensus
}

func (msg *RequestConsensus) MaxLength() uint32 {
	return 4
}

func (msg *RequestConsensus) Serialize(w io.Writer) error {
	if err := common.WriteUint32(w, msg.Height); err != nil {
		return err
	}

	return nil
}

func (msg *RequestConsensus) Deserialize(r io.Reader) error {
	var err error
	if msg.Height, err = common.ReadUint32(r); err != nil {
		return err
	}

	return nil
}
