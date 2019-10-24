// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"io"
)

const CRAppropriationVersion byte = 0x00

type CRAppropriation struct {
}

func (p *CRAppropriation) Data(version byte) []byte {
	return nil
}

func (p *CRAppropriation) Serialize(w io.Writer, version byte) error {
	return nil
}

func (p *CRAppropriation) Deserialize(r io.Reader, version byte) error {
	return nil
}
