// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/elanet/pact"
)

type SidechainIllegalData struct {
	Data payload.SidechainIllegalData
}

func (msg *SidechainIllegalData) CMD() string {
	return CmdSidechainIllegalData
}

func (msg *SidechainIllegalData) MaxLength() uint32 {
	return pact.MaxBlockContextSize
}

func (msg *SidechainIllegalData) Serialize(w io.Writer) error {
	if err := msg.Data.Serialize(w,
		payload.SidechainIllegalDataVersion); err != nil {
		return err
	}

	return nil
}

func (msg *SidechainIllegalData) Deserialize(r io.Reader) error {
	if err := msg.Data.Deserialize(r,
		payload.SidechainIllegalDataVersion); err != nil {
		return err
	}

	return nil
}
