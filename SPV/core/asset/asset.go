package asset

import (
	"io"
	"errors"

	"SPVWallet/core/serialization"
)

//AssetType
type AssetType byte

type AssetRecordType byte

//define the asset stucture in onchain DNA
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
	err := serialization.WriteVarString(w, a.Name)
	if err != nil {
		return errors.New("[Asset], Name serialize failed.")
	}
	err = serialization.WriteVarString(w, a.Description)
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
		return errors.New("[Asset], RecordType serialize failed.")
	}
	return nil
}

// Deserialize is the implement of SignableData interface.
func (a *Asset) Deserialize(r io.Reader) error {
	name, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("[Asset], Name deserialize failed.")
	}
	a.Name = name
	description, err := serialization.ReadVarString(r)
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
		return errors.New("[Asset], RecordType deserialize failed.")
	}
	return nil
}
