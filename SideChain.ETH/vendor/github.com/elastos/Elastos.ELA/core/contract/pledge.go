package contract

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"
)

func CreateDepositContractByPubKey(pubkey *crypto.PublicKey) (*Contract, error) {
	temp, err := pubkey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("[Contract],CreateDepositContractByPubKey failed.")
	}
	sb := program.NewProgramBuilder()
	sb.PushData(temp)
	sb.AddOp(vm.CHECKSIG)

	return &Contract{
		Code:   sb.ToArray(),
		Prefix: PrefixDeposit,
	}, nil
}

func CreateDepositContractByCode(code []byte) (*Contract, error) {
	return &Contract{
		Code:   code,
		Prefix: PrefixDeposit,
	}, nil
}

func PublicKeyToDepositProgramHash(pubKey []byte) (*common.Uint168, error) {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return nil, err
	}

	contract, err := CreateDepositContractByPubKey(publicKey)
	if err != nil {
		return nil, err
	}

	return contract.ToProgramHash(), nil
}
