package contract

import (
	"errors"
	"math/big"

	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"
)

func CreateMultiSigContractByPubKey(m int, pubkeys []*crypto.PublicKey) (*Contract, error) {
	if !(m >= 1 && m <= len(pubkeys) && len(pubkeys) <= 24) {
		return nil, nil //TODO: add panic
	}

	sb := pg.NewProgramBuilder()
	sb.PushNumber(big.NewInt(int64(m)))

	//sort pubkey
	crypto.SortPublicKeys(pubkeys)

	for _, pubkey := range pubkeys {
		temp, err := pubkey.EncodePoint(true)
		if err != nil {
			return nil, errors.New("[Contract],CreateMultiSigContractByPubKey failed.")
		}
		sb.PushData(temp)
	}

	sb.PushNumber(big.NewInt(int64(len(pubkeys))))
	sb.AddOp(vm.CHECKMULTISIG)

	return &Contract{
		RedeemScript: sb.ToArray(),
		HashPrefix:   PrefixMultisig,
	}, nil
}

func CreateMultiSigContractByCode(code []byte) (*Contract, error) {
	return &Contract{
		RedeemScript: code,
		HashPrefix:   PrefixMultisig,
	}, nil
}
