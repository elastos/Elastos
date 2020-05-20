// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"io"
)

const CRAssetsRectifyVersion byte = 0x00

type CRAssetsRectify struct {
}

func (p *CRAssetsRectify) Data(version byte) []byte {
	return nil
}

func (p *CRAssetsRectify) Serialize(w io.Writer, version byte) error {
	return nil
}

func (p *CRAssetsRectify) Deserialize(r io.Reader, version byte) error {
	return nil
}
