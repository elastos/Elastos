package contract

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"
)

func CreateStandardRedeemScript(pubKey *crypto.PublicKey) ([]byte, error) {
	temp, err := pubKey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("create standard redeem script, encode public key failed")
	}
	sb := program.NewProgramBuilder()
	sb.PushData(temp)
	sb.AddOp(vm.CHECKSIG)

	return sb.ToArray(), nil
}

func CreateStandardContract(pubKey *crypto.PublicKey) (*Contract, error) {
	redeemScript, err := CreateStandardRedeemScript(pubKey)
	if err != nil {
		return nil, err
	}

	return &Contract{
		Code:   redeemScript,
		Prefix: PrefixStandard,
	}, nil
}

func PublicKeyToStandardProgramHash(pubKey []byte) (*common.Uint168, error) {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return nil, err
	}

	contract, err := CreateStandardContract(publicKey)
	if err != nil {
		return nil, err
	}

	return contract.ToProgramHash(), nil
}

func PublicKeyToStandardCodeHash(pubKey []byte) (*common.Uint160, error) {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return nil, err
	}

	contract, err := CreateStandardContract(publicKey)
	if err != nil {
		return nil, err
	}

	return contract.ToCodeHash(), nil
}
