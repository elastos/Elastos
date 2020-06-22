// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"crypto/sha256"
	"errors"
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/contract/program"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
)

func RunPrograms(data []byte, programHashes []common.Uint168, programs []*Program) error {
	if len(programHashes) != len(programs) {
		return errors.New("the number of data hashes is different with number of programs")
	}

	for i, program := range programs {
		programHash := programHashes[i]
		prefixType := contract.GetPrefixType(programHash)

		// TODO: this implementation will be deprecated
		if prefixType == contract.PrefixCrossChain {
			if err := checkCrossChainSignatures(*program, data); err != nil {
				return err
			}
			continue
		}

		codeHash := common.ToCodeHash(program.Code)
		ownerHash := programHash.ToCodeHash()

		if !ownerHash.IsEqual(*codeHash) {
			return errors.New("the data hashes is different with corresponding program code")
		}

		if prefixType == contract.PrefixStandard || prefixType == contract.PrefixDeposit {
			if err := checkStandardSignature(*program, data); err != nil {
				return err
			}

		} else if prefixType == contract.PrefixMultiSig {
			if err := checkMultiSigSignatures(*program, data); err != nil {
				return err
			}
		} else {
			return errors.New("unknown signature type")
		}
	}

	return nil
}

func GetTxProgramHashes(tx *Transaction, references map[*Input]Output) ([]common.Uint168, error) {
	if tx == nil {
		return nil, errors.New("[Transaction],GetProgramHashes transaction is nil")
	}
	hashes := make([]common.Uint168, 0)
	uniqueHashes := make([]common.Uint168, 0)
	// add inputUTXO's transaction
	for _, output := range references {
		programHash := output.ProgramHash
		hashes = append(hashes, programHash)
	}
	for _, attribute := range tx.Attributes {
		if attribute.Usage == Script {
			dataHash, err := common.Uint168FromBytes(attribute.Data)
			if err != nil {
				return nil, errors.New("[Transaction], GetProgramHashes err")
			}
			hashes = append(hashes, *dataHash)
		}
	}

	//remove duplicated hashes
	unique := make(map[common.Uint168]bool)
	for _, v := range hashes {
		unique[v] = true
	}
	for k := range unique {
		uniqueHashes = append(uniqueHashes, k)
	}
	return uniqueHashes, nil
}

func checkStandardSignature(program Program, data []byte) error {
	if len(program.Parameter) != crypto.SignatureScriptLength {
		return errors.New("invalid signature length")
	}

	publicKey, err := crypto.DecodePoint(program.Code[1 : len(program.Code)-1])
	if err != nil {
		return err
	}

	return crypto.Verify(*publicKey, data, program.Parameter[1:])
}

func checkMultiSigSignatures(program Program, data []byte) error {
	code := program.Code
	// Get N parameter
	n := int(code[len(code)-2]) - crypto.PUSH1 + 1
	// Get M parameter
	m := int(code[0]) - crypto.PUSH1 + 1
	if m < 1 || m > n {
		return errors.New("invalid multi sign script code")
	}
	publicKeys, err := crypto.ParseMultisigScript(code)
	if err != nil {
		return err
	}

	return verifyMultisigSignatures(m, n, publicKeys, program.Parameter, data)
}

func checkCrossChainSignatures(program Program, data []byte) error {
	code := program.Code
	// Get N parameter
	n := int(code[len(code)-2]) - crypto.PUSH1 + 1
	// Get M parameter
	m := int(code[0]) - crypto.PUSH1 + 1
	if m < 1 || m > n || n != int(DefaultLedger.Arbitrators.GetCrossChainArbitersCount()) ||
		m <= int(DefaultLedger.Arbitrators.GetCrossChainArbitersMajorityCount()) {
		return errors.New("invalid multi sign script code")
	}
	publicKeys, err := crypto.ParseCrossChainScript(code)
	if err != nil {
		return err
	}

	return verifyMultisigSignatures(m, n, publicKeys, program.Parameter, data)
}

func verifyMultisigSignatures(m, n int, publicKeys [][]byte, signatures, data []byte) error {
	if len(publicKeys) != n {
		return errors.New("invalid multi sign public key script count")
	}
	if len(signatures)%crypto.SignatureScriptLength != 0 {
		return errors.New("invalid multi sign signatures, length not match")
	}
	if len(signatures)/crypto.SignatureScriptLength < m {
		return errors.New("invalid signatures, not enough signatures")
	}
	if len(signatures)/crypto.SignatureScriptLength > n {
		return errors.New("invalid signatures, too many signatures")
	}

	var verified = make(map[common.Uint256]struct{})
	for i := 0; i < len(signatures); i += crypto.SignatureScriptLength {
		// Remove length byte
		sign := signatures[i : i+crypto.SignatureScriptLength][1:]
		// Match public key with signature
		for _, publicKey := range publicKeys {
			pubKey, err := crypto.DecodePoint(publicKey[1:])
			if err != nil {
				return err
			}
			err = crypto.Verify(*pubKey, data, sign)
			if err == nil {
				hash := sha256.Sum256(publicKey)
				if _, ok := verified[hash]; ok {
					return errors.New("duplicated signatures")
				}
				verified[hash] = struct{}{}
				break // back to public keys loop
			}
		}
	}
	// Check signatures count
	if len(verified) < m {
		return errors.New("matched signatures not enough")
	}

	return nil
}

func SortPrograms(programs []*Program) {
	sort.Sort(byHash(programs))
}

type byHash []*Program

func (p byHash) Len() int      { return len(p) }
func (p byHash) Swap(i, j int) { p[i], p[j] = p[j], p[i] }
func (p byHash) Less(i, j int) bool {
	hashi := common.ToCodeHash(p[i].Code)
	hashj := common.ToCodeHash(p[j].Code)
	return hashi.Compare(*hashj) < 0
}
