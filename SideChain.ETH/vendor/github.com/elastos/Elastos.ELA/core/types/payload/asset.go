package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

//AssetType
type AssetType byte

const (
	Token AssetType = 0x00
	Share AssetType = 0x01
)

const (
	MaxPrecision = 8
	MinPrecision = 0
)

type AssetRecordType byte

const (
	Unspent AssetRecordType = 0x00
	Balance AssetRecordType = 0x01
)

//registered asset will be assigned to contract address
type Asset struct {
	Name        string
	Description string
	Precision   byte
	AssetType   AssetType
	RecordType  AssetRecordType
}

// Serialize is the implement of SignableData interface.
func (a *Asset) Serialize(w io.Writer) error {
	err := common.WriteVarString(w, a.Name)
	if err != nil {
		return errors.New("[Asset], Name serialize failed.")
	}
	err = common.WriteVarString(w, a.Description)
	if err != nil {
		return errors.New("[Asset], Description serialize failed.")
	}
	_, err = w.Write([]byte{byte(a.Precision)})
	if err != nil {
		return errors.New("[Asset], Precision serialize failed.")
	}
	_, err = w.Write([]byte{byte(a.AssetType)})
	if err != nil {
		return errors.New("[Asset], AssetType serialize failed.")
	}
	_, err = w.Write([]byte{byte(a.RecordType)})
	if err != nil {
		return errors.New("[Asset], Type serialize failed.")
	}
	return nil
}

// Deserialize is the implement of SignableData interface.
func (a *Asset) Deserialize(r io.Reader) error {
	name, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[Asset], Name deserialize failed.")
	}
	a.Name = name
	description, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[Asset], Description deserialize failed.")
	}
	a.Description = description
	p := make([]byte, 1)
	n, err := r.Read(p)
	if n > 0 {
		a.Precision = p[0]
	} else {
		return errors.New("[Asset], Precision deserialize failed.")
	}
	n, err = r.Read(p)
	if n > 0 {
		a.AssetType = AssetType(p[0])
	} else {
		return errors.New("[Asset], AssetType deserialize failed.")
	}
	n, err = r.Read(p)
	if n > 0 {
		a.RecordType = AssetRecordType(p[0])
	} else {
		return errors.New("[Asset], Type deserialize failed.")
	}
	return nil
}

func (a *Asset) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	a.Serialize(buf)
	return common.Sha256D(buf.Bytes())
}
