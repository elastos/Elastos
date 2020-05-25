// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package payload

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const RecordVersion byte = 0x00

type Record struct {
	Type    string
	Content []byte
}

func (a *Record) Data(version byte) []byte {
	//TODO: implement RegisterRecord.Data()
	return []byte{0}
}

// Serialize is the implement of SignableData interface.
func (a *Record) Serialize(w io.Writer, version byte) error {
	err := common.WriteVarString(w, a.Type)
	if err != nil {
		return errors.New("[RecordDetail], Type serialize failed.")
	}
	err = common.WriteVarBytes(w, a.Content)
	if err != nil {
		return errors.New("[RecordDetail], Record serialize failed.")
	}
	return nil
}

// Deserialize is the implement of SignableData interface.
func (a *Record) Deserialize(r io.Reader, version byte) error {
	var err error
	a.Type, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RecordDetail], Type deserialize failed.")
	}
	a.Content, err = common.ReadVarBytes(r, MaxPayloadDataSize,
		"payload record data")
	if err != nil {
		return errors.New("[RecordDetail], Record deserialize failed.")
	}
	return nil
}
