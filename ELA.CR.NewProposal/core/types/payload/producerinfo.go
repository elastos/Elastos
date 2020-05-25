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

const ProducerInfoVersion byte = 0x00

type ProducerInfo struct {
	OwnerPublicKey []byte
	NodePublicKey  []byte
	NickName       string
	Url            string
	Location       uint64
	NetAddress     string
	Signature      []byte
}

func (a *ProducerInfo) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *ProducerInfo) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[ProducerInfo], Signature serialize failed")
	}

	return nil
}

func (a *ProducerInfo) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.OwnerPublicKey)
	if err != nil {
		return errors.New("[ProducerInfo], owner publicKey serialize failed")
	}

	err = common.WriteVarBytes(w, a.NodePublicKey)
	if err != nil {
		return errors.New("[ProducerInfo], node publicKey serialize failed")
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[ProducerInfo], nickname serialize failed")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[ProducerInfo], url serialize failed")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[ProducerInfo], location serialize failed")
	}

	err = common.WriteVarString(w, a.NetAddress)
	if err != nil {
		return errors.New("[ProducerInfo], address serialize failed")
	}

	return nil
}

func (a *ProducerInfo) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	a.Signature, err = common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[ProducerInfo], signature deserialize failed")
	}

	return nil
}

func (a *ProducerInfo) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	a.OwnerPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "own public key")
	if err != nil {
		return errors.New("[ProducerInfo], owner publicKey deserialize failed")
	}

	a.NodePublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "node public key")
	if err != nil {
		return errors.New("[ProducerInfo], node publicKey deserialize failed")
	}

	a.NickName, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[ProducerInfo], nickName deserialize failed")
	}

	a.Url, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[ProducerInfo], url deserialize failed")
	}

	a.Location, err = common.ReadUint64(r)
	if err != nil {
		return errors.New("[ProducerInfo], location deserialize failed")
	}

	a.NetAddress, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[ProducerInfo], address deserialize failed")
	}

	return nil
}
