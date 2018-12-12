package contract

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"
)

func CreatePledgeContractByPubKey(pubkey *crypto.PublicKey) (*Contract, error) {
	temp, err := pubkey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("[Contract],CreatePledgeContractByPubKey failed.")
	}
	sb := program.NewProgramBuilder()
	sb.PushData(temp)
	sb.AddOp(vm.CHECKSIG)

	return &Contract{
		Code:       sb.ToArray(),
		HashPrefix: PrefixPledge,
	}, nil
}

func CreatePledgeContractByCode(code []byte) (*Contract, error) {
	return &Contract{
		Code:       code,
		HashPrefix: PrefixPledge,
	}, nil
}

func PublicKeyToPledgeProgramHash(pubKey []byte) (*common.Uint168, error) {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return nil, err
	}

	contract, err := CreatePledgeContractByPubKey(publicKey)
	if err != nil {
		return nil, err
	}

	return contract.ToProgramHash()
}
