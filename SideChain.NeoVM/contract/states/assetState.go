package states

import (
	"io"
	"errors"
	"bytes"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	MaxByteTypeSize = 1
)

type AssetState struct {
	StateBase
	AssetId    common.Uint256
	AssetType  types.AssetType
	Name       string
	Amount     common.Fixed64
	Avaliable  common.Fixed64
	Precision  byte
	FeeMode    byte
	Fee        common.Fixed64
	FeeAddress common.Uint168
	Owner      *crypto.PublicKey
	Admin      common.Uint168
	Issuer     common.Uint168
	Expiration uint32
	IsFrozen   bool
}

func (assetState *AssetState) Serialize(w io.Writer) error {
	assetState.StateBase.Serialize(w)
	assetState.AssetId.Serialize(w)
	common.WriteVarBytes(w, []byte{byte(assetState.AssetType)})
	common.WriteVarString(w, assetState.Name)
	assetState.Amount.Serialize(w)
	assetState.Avaliable.Serialize(w)
	common.WriteVarBytes(w, []byte{assetState.Precision})
	common.WriteVarBytes(w, []byte{assetState.FeeMode})
	assetState.Fee.Serialize(w)
	assetState.FeeAddress.Serialize(w)
	assetState.Owner.Serialize(w)
	assetState.Admin.Serialize(w)
	assetState.Issuer.Serialize(w)
	common.WriteUint32(w, assetState.Expiration)
	var v uint8 = 0
	if assetState.IsFrozen {
		v = 1
	}
	common.WriteUint8(w, v)
	return nil
}

func (assetState *AssetState) Deserialize(r io.Reader) error {
	u256 := new(common.Uint256)
	u168 := new(common.Uint168)
	f := new(common.Fixed64)
	pubKey := crypto.PublicKey{}
	stateBase := new(StateBase)
	err := stateBase.Deserialize(r)
	if err != nil {
		return err
	}
	assetState.StateBase = *stateBase

	err = u256.Deserialize(r)
	if err != nil {
		return errors.New("AssetState AssetId Deserialize fail.")
	}
	assetState.AssetId = *u256
	assetTypes, err := common.ReadVarBytes(r, MaxByteTypeSize, "AssetState Deserialize, assetTypes")
	if err != nil {
		return errors.New("AssetState AssetType Deserialize fail.")
	}
	assetState.AssetType = types.AssetType(assetTypes[0])

	name, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("AssetState Name Deserialize fail.")
	}
	assetState.Name = name

	err = f.Deserialize(r)
	if err != nil {
		return errors.New("AssetState Amount Deserialize fail.")
	}
	assetState.Amount = *f

	err = f.Deserialize(r)
	if err != nil {
		return errors.New("AssetState Available Deserialize fail.")
	}
	assetState.Avaliable = *f

	precisions, err := common.ReadVarBytes(r, MaxByteTypeSize, "AssetState Deserialize precisions")
	if err != nil {
		return errors.New("AssetState Precision Deserialize fail.")
	}
	assetState.Precision = precisions[0]

	feeModes, err := common.ReadVarBytes(r, MaxByteTypeSize, "AssetState Deserialize feeModes")
	if err != nil {
		return errors.New("AssetState FeeMode Deserialize fail.")
	}
	assetState.FeeMode = feeModes[0]

	err = f.Deserialize(r)
	if err != nil {
		return errors.New("AssetState Fee Deserialize fail.")
	}
	assetState.Fee = *f

	err = u168.Deserialize(r)
	if err != nil {
		return errors.New("AssetState FeeAddress Deserialize fail.")
	}
	assetState.FeeAddress = *u168

	err = pubKey.Deserialize(r)
	if err != nil {
		return errors.New("AssetState Owner Deserialize fail.")
	}
	assetState.Owner = &pubKey

	err = u168.Deserialize(r)
	if err != nil {
		return errors.New("AssetState Admin Deserialize fail.")
	}
	assetState.Admin = *u168

	err = u168.Deserialize(r)
	if err != nil {
		return errors.New("AssetState Issuer Deserialize fail.")
	}
	assetState.Issuer = *u168

	expiration, err := common.ReadUint32(r)
	if err != nil {
		return errors.New("AssetState Expiration Deserialize fail.")
	}
	assetState.Expiration = expiration

	isFrozon, err := common.ReadUint8(r)
	if err != nil {
		return errors.New("AssetState IsFrozon Deserialize fail.")
	}
	assetState.IsFrozen = false
	if isFrozon == 1 {
		assetState.IsFrozen = true
	}
	return nil
}

func (assetState *AssetState) Bytes() []byte {
	b := new(bytes.Buffer)
	assetState.Serialize(b)
	return b.Bytes()
}
