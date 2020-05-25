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

const CRInfoVersion byte = 0x00
const CRInfoDIDVersion byte = 0x01

// CRInfo defines the information of CR.
type CRInfo struct {
	Code      []byte
	CID       common.Uint168
	DID       common.Uint168
	NickName  string
	Url       string
	Location  uint64
	Signature []byte
}

func (a *CRInfo) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *CRInfo) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[CRInfo], Signature serialize failed")
	}

	return nil
}

func (a *CRInfo) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.Code)
	if err != nil {
		return errors.New("[CRInfo], code serialize failed")
	}

	if err = a.CID.Serialize(w); err != nil {
		return errors.New("[CRInfo], CID serialize failed")
	}

	if version > CRInfoVersion {
		if err = a.DID.Serialize(w); err != nil {
			return errors.New("[CRInfo], DID serialize failed")
		}
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[CRInfo], nickname serialize failed")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[CRInfo], url serialize failed")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[CRInfo], location serialize failed")
	}

	return nil
}

func (a *CRInfo) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	a.Signature, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "signature")
	if err != nil {
		return errors.New("[CRInfo], signature deserialize failed")
	}

	return nil
}

func (a *CRInfo) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	a.Code, err = common.ReadVarBytes(r, crypto.MaxMultiSignCodeLength, "code")
	if err != nil {
		return errors.New("[CRInfo], code deserialize failed")
	}

	if err = a.CID.Deserialize(r); err != nil {
		return errors.New("[CRInfo], CID deserialize failed")
	}

	if version > CRInfoVersion {
		if err = a.DID.Deserialize(r); err != nil {
			return errors.New("[CRInfo], DID deserialize failed")
		}
	}

	a.NickName, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[CRInfo], nickName deserialize failed")
	}

	a.Url, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[CRInfo], url deserialize failed")
	}

	a.Location, err = common.ReadUint64(r)
	if err != nil {
		return errors.New("[CRInfo], location deserialize failed")
	}

	return nil
}
func (a *CRInfo) GetCodeHash() common.Uint160 {
	return *common.ToCodeHash(a.Code)
}
