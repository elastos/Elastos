// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package contract

import (
	"errors"
	"math/big"

	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"
)

func CreateMultiSigRedeemScript(m int, pubkeys []*crypto.PublicKey) ([]byte, error) {
	if len(pubkeys) == 0 {
		return nil, errors.New("public keys is nil")
	}
	for _, pk := range pubkeys {
		if nil == pk {
			return nil, errors.New("public keys has nil public key")
		}
	}
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
			return nil, errors.New("[Contract],CreateMultiSigContract failed.")
		}
		sb.PushData(temp)
	}

	sb.PushNumber(big.NewInt(int64(len(pubkeys))))
	sb.AddOp(vm.CHECKMULTISIG)

	return sb.ToArray(), nil
}

func CreateMultiSigContract(m int, pubkeys []*crypto.PublicKey) (*Contract, error) {
	redeemScript, err := CreateMultiSigRedeemScript(m, pubkeys)
	if err != nil {
		return nil, err
	}

	return &Contract{
		Code:   redeemScript,
		Prefix: PrefixMultiSig,
	}, nil
}

func CreateMultiSigContractByCode(code []byte) (*Contract, error) {
	return &Contract{
		Code:   code,
		Prefix: PrefixMultiSig,
	}, nil
}
