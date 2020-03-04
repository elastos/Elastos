// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"io"
)

const CRCAppropriationVersion byte = 0x00

type CRCAppropriation struct {
}

func (p *CRCAppropriation) Data(version byte) []byte {
	return nil
}

func (p *CRCAppropriation) Serialize(w io.Writer, version byte) error {
	return nil
}

func (p *CRCAppropriation) Deserialize(r io.Reader, version byte) error {
	return nil
}
