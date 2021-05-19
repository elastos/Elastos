// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package account

import (
	"encoding/hex"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	MAINACCOUNT      = "main-account"
	SUBACCOUNT       = "sub-account"
	KeystoreFileName = "keystore.dat"
	KeystoreVersion  = "1.0.0"

	MaxSignalQueueLen = 5
)

var IDReverse, _ = hex.DecodeString("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0")
var SystemAssetID, _ = common.Uint256FromBytes(common.BytesReverse(IDReverse))

func GetSigners(code []byte) ([]*common.Uint160, error) {
	publicKeys, err := crypto.ParseMultisigScript(code)
	if err != nil {
		return nil, err
	}

	var signers []*common.Uint160
	for _, publicKey := range publicKeys {
		hash, err := contract.PublicKeyToStandardCodeHash(publicKey[1:])
		if err != nil {
			return nil, err
		}
		signers = append(signers, hash)
	}

	return signers, nil
}

func GetCorssChainSigners(code []byte) ([]*common.Uint160, error) {
	publicKeys, err := crypto.ParseCrossChainScript(code)
	if err != nil {
		return nil, err
	}

	var signers []*common.Uint160
	for _, publicKey := range publicKeys {
		hash, err := contract.PublicKeyToStandardCodeHash(publicKey[1:])
		if err != nil {
			return nil, err
		}
		signers = append(signers, hash)
	}

	return signers, nil
}
