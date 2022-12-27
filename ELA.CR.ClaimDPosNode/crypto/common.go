// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package crypto

import (
	"errors"
	"github.com/elastos/Elastos.ELA/common"
)

const (
	PUSH1 = 0x51

	// encoded public key length 0x21 || encoded public key (33 bytes) || OP_CHECKSIG(0xac)
	PublicKeyScriptLength = 35

	// signature length(0x40) || 64 bytes signature
	SignatureScriptLength = 65

	// max signature length(0x40) || 1000 signature (64 bytes * 1000)
	MaxSignatureScriptLength = 64001

	// 1byte m || 2 encoded public keys with leading 0x40 (34 bytes * 2) ||
	// 1byte n + 1byte OP_CHECKMULTISIG
	MinMultiSignCodeLength = 71

	// 1byte m || 1000 encoded public keys with leading 0x40 (34 bytes * 1000) ||
	// 1byte n + 1byte OP_CHECKMULTISIG
	MaxMultiSignCodeLength = 34003
)

func ParseMultisigScript(code []byte) ([][]byte, error) {
	if len(code) < MinMultiSignCodeLength || code[len(code)-1] != common.MULTISIG {
		return nil, errors.New("not a valid multi sign transaction code, length not enough")
	}
	return parsePublicKeys(code)
}

func ParseCrossChainScript(code []byte) ([][]byte, error) {
	if len(code) < MinMultiSignCodeLength || code[len(code)-1] != common.CROSSCHAIN {
		return nil, errors.New("not a valid cross chain transaction code, length not enough")
	}
	return parsePublicKeys(code)
}

func ParseCrossChainScriptV1(code []byte) ([][]byte, int, int, error) {
	if len(code) < MinMultiSignCodeLength || code[len(code)-1] != common.CROSSCHAIN {
		return nil, 0, 0, errors.New("not a valid cross chain transaction code, length not enough")
	}
	m, n := parseMAndN(code)
	pks, err := parsePublicKeys(code)
	return pks, m, n, err
}

func parseMAndN(code []byte) (int, int) {
	// Get N parameter
	n := int(code[len(code)-2]) - PUSH1 + 1
	// Get M parameter
	m := int(code[0]) - PUSH1 + 1
	return m, n
}

func parsePublicKeys(code []byte) ([][]byte, error) {
	// remove last byte MULTISIG
	code = code[:len(code)-1]
	// remove m
	code = code[1:]
	// remove n
	code = code[:len(code)-1]
	if len(code)%(PublicKeyScriptLength-1) != 0 {
		return nil, errors.New("not a valid transaction code, length not match")
	}

	var publicKeys [][]byte
	i := 0
	for i < len(code) {
		script := make([]byte, PublicKeyScriptLength-1)
		copy(script, code[i:i+PublicKeyScriptLength-1])
		i += PublicKeyScriptLength - 1
		publicKeys = append(publicKeys, script)
	}
	return publicKeys, nil
}

func GetScriptType(script []byte) (byte, error) {
	if len(script) != PublicKeyScriptLength && len(script) < MinMultiSignCodeLength {
		return 0, errors.New("invalid redeem script, not a standard or multi sign type")
	}
	return script[len(script)-1], nil
}

func GetM(code []byte) (uint, error) {
	scriptType, err := GetScriptType(code)
	if err != nil {
		return 0, err
	}
	if scriptType != common.MULTISIG {
		return 0, errors.New("not a multi-signature script")
	}
	return getM(code), nil
}

func getM(code []byte) uint {
	return uint(code[0] - PUSH1 + 1)
}

func GetSignStatus(code, param []byte) (haveSign, needSign int, err error) {
	scriptType, err := GetScriptType(code)
	if err != nil {
		return -1, -1, err
	}

	if scriptType == common.STANDARD {
		signed := len(param) / SignatureScriptLength
		return signed, 1, nil

	} else if scriptType == common.MULTISIG {

		haveSign = len(param) / SignatureScriptLength

		return haveSign, int(getM(code)), nil
	}

	return -1, -1, errors.New("invalid redeem script type")
}

func AppendSignature(signerIndex int, signature, data, code, param []byte) ([]byte, error) {
	// Check if singer already signed
	if param != nil {
		publicKeys, err := ParseMultisigScript(code)
		if err != nil {
			return nil, err
		}
		for i := 0; i < len(param); i += SignatureScriptLength {
			// Remove length byte
			sign := param[i : i+SignatureScriptLength][1:]
			publicKey := publicKeys[signerIndex][1:]
			pubKey, err := DecodePoint(publicKey)
			if err != nil {
				return nil, err
			}
			err = Verify(*pubKey, data, sign)
			if err == nil {
				return nil, errors.New("signer already signed")
			}
		}
	}

	// Append new signature
	param = append(param, byte(len(signature)))
	return append(param, signature...), nil
}
