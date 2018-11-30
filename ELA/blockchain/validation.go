package blockchain

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/crypto"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

func RunPrograms(data []byte, programHashes []Uint168, programs []*Program) error {
	if len(programHashes) != len(programs) {
		return errors.New("The number of data hashes is different with number of programs.")
	}

	for i, program := range programs {
		codeHash, err := common.ToCodeHash(program.Code)
		if err != nil {
			return err
		}

		programHash := programHashes[i]
		ownerHash := common.Uint160ParseFromUint168(programHash)

		if !ownerHash.IsEqual(codeHash) && programHash[0] != PrefixCrossChain {
			return errors.New("The data hashes is different with corresponding program code.")
		}

		if programHash[0] == PrefixStandard {
			if err := checkStandardSignature(*program, data); err != nil {
				return err
			}

		} else if programHash[0] == PrefixMultisig {
			if err = checkMultiSigSignatures(*program, data); err != nil {
				return err
			}

		} else if programHash[0] == PrefixCrossChain {
			if err = checkCrossChainSignatures(*program, data); err != nil {
				return err
			}

		} else {
			return errors.New("unknown signature type")
		}
	}

	return nil
}

func GetTxProgramHashes(tx *Transaction, references map[*Input]*Output) ([]Uint168, error) {
	if tx == nil {
		return nil, errors.New("[Transaction],GetProgramHashes transaction is nil.")
	}
	hashes := make([]Uint168, 0)
	uniqueHashes := make([]Uint168, 0)
	// add inputUTXO's transaction
	for _, output := range references {
		programHash := output.ProgramHash
		hashes = append(hashes, programHash)
	}
	for _, attribute := range tx.Attributes {
		if attribute.Usage == Script {
			dataHash, err := Uint168FromBytes(attribute.Data)
			if err != nil {
				return nil, errors.New("[Transaction], GetProgramHashes err.")
			}
			hashes = append(hashes, *dataHash)
		}
	}

	//remove dupilicated hashes
	unique := make(map[Uint168]bool)
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
		return errors.New("Invalid signature length")
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
	if m < 1 || m > n {
		return errors.New("invalid multi sign script code")
	}
	publicKeys, err := crypto.ParseCrossChainScript(code)
	if err != nil {
		return err
	}

	if err := checkCrossChainArbitrators(publicKeys); err != nil {
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

	var verified = make(map[Uint256]struct{})
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

func checkCrossChainArbitrators(publicKeys [][]byte) error {
	arbitrators := DefaultLedger.Arbitrators.GetArbitrators()
	if len(arbitrators) != len(publicKeys) {
		return errors.New("Invalid arbitrator count.")
	}

	for _, arbitrator := range arbitrators {
		found := false
		for _, pk := range publicKeys {
			if bytes.Equal(arbitrator, pk[1:]) {
				found = true
				break
			}
		}

		if !found {
			return errors.New("Invalid cross chain arbitrators")
		}
	}
	return nil
}

func SortPrograms(programs []*Program) (err error) {
	defer func() {
		if code := recover(); code != nil {
			err = fmt.Errorf("invalid program code %x", code)
		}
	}()
	sort.Sort(byHash(programs))
	return err
}

type byHash []*Program

func (p byHash) Len() int      { return len(p) }
func (p byHash) Swap(i, j int) { p[i], p[j] = p[j], p[i] }
func (p byHash) Less(i, j int) bool {
	hashi, err := common.ToCodeHash(p[i].Code)
	if err != nil {
		panic(p[i].Code)
	}
	hashj, err := common.ToCodeHash(p[j].Code)
	if err != nil {
		panic(p[j].Code)
	}
	return hashi.Compare(hashj) < 0
}
