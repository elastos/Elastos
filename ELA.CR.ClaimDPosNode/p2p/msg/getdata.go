// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure GetData implement p2p.Message interface.
var _ p2p.Message = (*GetData)(nil)

type GetData struct {
	Inv
}

func NewGetData() *GetData {
	return &GetData{}
}

func (msg *GetData) CMD() string {
	return p2p.CmdGetData
}
