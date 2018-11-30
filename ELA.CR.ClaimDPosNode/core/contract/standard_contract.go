package contract

import (
	"errors"

	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

func CreateStandardContractByPubKey(pubkey *crypto.PublicKey) (*Contract, error) {
	temp, err := pubkey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("[Contract],CreateStandardContractByPubKey failed.")
	}
	sb := program.NewProgramBuilder()
	sb.PushData(temp)
	sb.AddOp(vm.CHECKSIG)

	return &Contract{
		RedeemScript: sb.ToArray(),
		HashPrefix:   common.PrefixStandard,
	}, nil
}

func CreateStandardContractByCode(code []byte) (*Contract, error) {
	return &Contract{
		RedeemScript: code,
		HashPrefix:   common.PrefixStandard,
	}, nil
}

func PublicKeyToStandardProgramHash(pubKey []byte) (*common.Uint168, error) {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return nil, err
	}

	contract, err := CreateStandardContractByPubKey(publicKey)
	if err != nil {
		return nil, err
	}

	return contract.ToProgramHash()
}
