// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure MemPool implement p2p.Message interface.
var _ p2p.Message = (*MemPool)(nil)

type MemPool struct{ empty }

func (msg *MemPool) CMD() string {
	return p2p.CmdMemPool
}
