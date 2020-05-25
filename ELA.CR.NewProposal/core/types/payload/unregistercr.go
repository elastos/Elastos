// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const UnregisterCRVersion byte = 0x00

type UnregisterCR struct {
	CID       common.Uint168
	Signature []byte
}

func (a *UnregisterCR) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *UnregisterCR) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[UnregisterCR], Signature serialize failed")
	}

	return nil
}

func (a *UnregisterCR) SerializeUnsigned(w io.Writer, version byte) error {
	if err := a.CID.Serialize(w); err != nil {
		return errors.New("[UnregisterCR], CID serialize failed")
	}
	return nil
}

func (a *UnregisterCR) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	a.Signature, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "signature")
	if err != nil {
		return errors.New("[UnregisterCR], signature deserialize failed")
	}

	return nil
}

func (a *UnregisterCR) DeserializeUnsigned(r io.Reader, version byte) error {
	if err := a.CID.Deserialize(r); err != nil {
		return errors.New("[UnregisterCR], CID deserialize failed")
	}
	return nil
}
