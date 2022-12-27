package types

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const RecordPayloadVersion byte = 0x00

type PayloadRecord struct {
	RecordType string
	RecordData []byte
}

func (a *PayloadRecord) Data(version byte) []byte {
	//TODO: implement RegisterRecord.Data()
	return []byte{0}
}

// Serialize is the implement of SignableData interface.
func (a *PayloadRecord) Serialize(w io.Writer, version byte) error {
	err := common.WriteVarString(w, a.RecordType)
	if err != nil {
		return errors.New("[RecordDetail], RecordType serialize failed.")
	}
	err = common.WriteVarBytes(w, a.RecordData)
	if err != nil {
		return errors.New("[RecordDetail], RecordData serialize failed.")
	}
	return nil
}

// Deserialize is the implement of SignableData interface.
func (a *PayloadRecord) Deserialize(r io.Reader, version byte) error {
	var err error
	a.RecordType, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RecordDetail], RecordType deserialize failed.")
	}
	a.RecordData, err = common.ReadVarBytes(r, MaxPayloadDataSize,
		"payload record data")
	if err != nil {
		return errors.New("[RecordDetail], RecordData deserialize failed.")
	}
	return nil
}
