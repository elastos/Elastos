package core

import (
	"io"
	"errors"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type PayloadRegisterAsset struct {
	Asset      Asset
	Amount     Fixed64
	Controller Uint168
}

func (a *PayloadRegisterAsset) Data(version byte) []byte {
	//TODO: implement RegisterAsset.Data()
	return []byte{0}

}

func (a *PayloadRegisterAsset) Serialize(w io.Writer, version byte) error {
	err := a.Asset.Serialize(w)
	if err != nil {
		return errors.New("[RegisterAsset], Asset Serialize failed.")
	}
	err = a.Amount.Serialize(w)
	if err != nil {
		return errors.New("[RegisterAsset], Ammount Serialize failed.")
	}
	err = a.Controller.Serialize(w)
	if err != nil {
		return errors.New("[RegisterAsset], Controller Serialize failed.")
	}
	return nil
}

func (a *PayloadRegisterAsset) Deserialize(r io.Reader, version byte) error {
	//asset
	err := a.Asset.Deserialize(r)
	if err != nil {
		return errors.New("[RegisterAsset], Asset Deserialize failed.")
	}

	//Value
	err = a.Amount.Deserialize(r)
	if err != nil {
		return errors.New("[RegisterAsset], Ammount Deserialize failed.")
	}

	//Controller *Uint168
	err = a.Controller.Deserialize(r)
	if err != nil {
		return errors.New("[RegisterAsset], Ammount Deserialize failed.")
	}
	return nil
}
