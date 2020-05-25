// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const UpdateVersionVersion byte = 0x00

type UpdateVersion struct {
	StartHeight uint32
	EndHeight   uint32
}

func (u *UpdateVersion) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := u.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (u *UpdateVersion) Serialize(w io.Writer, version byte) error {
	if err := common.WriteUint32(w, u.StartHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, u.EndHeight); err != nil {
		return err
	}

	return nil
}

func (u *UpdateVersion) Deserialize(r io.Reader, version byte) (err error) {
	if u.StartHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	if u.EndHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	return nil
}
