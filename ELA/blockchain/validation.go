package blockchain

import (
	"bytes"
	"crypto/sha256"
	"errors"

	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/sidechain"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

func VerifySignature(tx *Transaction) error {
	hashes, err := GetTxProgramHashes(tx)
	if err != nil {
		return err
	}

	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)

	return RunPrograms(buf.Bytes(), hashes, tx.Programs)
}

func RunPrograms(data []byte, hashes []common.Uint168, programs []*Program) error {
	if len(hashes) != len(programs) {
		return errors.New("The number of data hashes is different with number of programs.")
	}

	for i := 0; i < len(programs); i++ {

		code := programs[i].Code
		param := programs[i].Parameter

		programHash, err := crypto.ToProgramHash(code)
		if err != nil {
			return err
		}

		// Get transaction type
		signType, err := crypto.GetScriptType(code)
		if err != nil {
			return err
		}

		if !hashes[i].IsEqual(*programHash) && signType != common.CROSSCHAIN {
			return errors.New("The data hashes is different with corresponding program code.")
		}

		if signType == common.STANDARD {
			// Remove length byte and sign type byte
			publicKeyBytes := code[1 : len(code)-1]
			if err = checkStandardSignature(publicKeyBytes, data, param); err != nil {
				return err
			}

		} else if signType == common.MULTISIG {
			publicKeys, err := crypto.ParseMultisigScript(code)
			if err != nil {
				return err
			}
			if err = checkMultiSignSignatures(code, param, data, publicKeys); err != nil {
				return err
			}

		} else if signType == common.CROSSCHAIN {
			publicKeys, err := crypto.ParseCrossChainScript(code)
			if err != nil {
				return err
			}
			if err = checkCrossChainArbitrators(data, publicKeys); err != nil {
				return err
			}
			if err = checkMultiSignSignatures(code, param, data, publicKeys); err != nil {
				return err
			}

		} else {
			return errors.New("unknown signature type")
		}
	}

	return nil
}

func GetTxProgramHashes(tx *Transaction) ([]common.Uint168, error) {
	if tx == nil {
		return nil, errors.New("[Transaction],GetProgramHashes transaction is nil.")
	}
	hashes := make([]common.Uint168, 0)
	uniqueHashes := make([]common.Uint168, 0)
	// add inputUTXO's transaction
	references, err := DefaultLedger.Store.GetTxReference(tx)
	if err != nil {
		return nil, errors.New("[Transaction], GetProgramHashes failed.")
	}
	for _, output := range references {
		programHash := output.ProgramHash
		hashes = append(hashes, programHash)
	}
	for _, attribute := range tx.Attributes {
		if attribute.Usage == Script {
			dataHash, err := common.Uint168FromBytes(attribute.Data)
			if err != nil {
				return nil, errors.New("[Transaction], GetProgramHashes err.")
			}
			hashes = append(hashes, *dataHash)
		}
	}
	switch tx.TxType {
	case RegisterAsset:
	case TransferAsset:
	case Record:
	case Deploy:
	case SideMining:
	default:
	}

	//remove dupilicated hashes
	unique := make(map[common.Uint168]bool)
	for _, v := range hashes {
		unique[v] = true
	}
	for k := range unique {
		uniqueHashes = append(uniqueHashes, k)
	}
	common.SortProgramHashes(uniqueHashes)
	return uniqueHashes, nil
}

func checkStandardSignature(publicKeyBytes, content, signature []byte) error {
	if len(signature) != crypto.SignatureScriptLength {
		return errors.New("Invalid signature length")
	}

	publicKey, err := crypto.DecodePoint(publicKeyBytes)
	if err != nil {
		return err
	}

	return crypto.Verify(*publicKey, content, signature[1:])
}

func checkMultiSignSignatures(code, param, content []byte, publicKeys [][]byte) error {
	// Get N parameter
	n := int(code[len(code)-2]) - crypto.PUSH1 + 1
	// Get M parameter
	m := int(code[0]) - crypto.PUSH1 + 1
	if m < 1 || m > n {
		return errors.New("invalid multi sign script code")
	}
	if len(publicKeys) != n {
		return errors.New("invalid multi sign public key script count")
	}

	var verified = make(map[common.Uint256]struct{})
	for i := 0; i < len(param); i += crypto.SignatureScriptLength {
		// Remove length byte
		sign := param[i: i+crypto.SignatureScriptLength][1:]
		// Get signature index, if signature exists index will not be -1
		for _, publicKey := range publicKeys {
			pubKey, err := crypto.DecodePoint(publicKey[1:])
			if err != nil {
				return err
			}
			err = crypto.Verify(*pubKey, content, sign)
			if err == nil {
				pkBytes := append(pubKey.X.Bytes(), pubKey.Y.Bytes()...)
				hash := sha256.Sum256(pkBytes)
				if _, ok := verified[hash]; ok {
					return errors.New("duplicated signatures")
				}
				verified[hash] = struct{}{}
				break // back to public keys loop
			}
		}
	}
	// Check signature count
	if len(verified) != m {
		return errors.New("invalid signature count")
	}

	return nil
}

func checkCrossChainArbitrators(data []byte, publicKeys [][]byte) error {
	var tx Transaction
	err := tx.DeserializeUnsigned(bytes.NewReader(data))
	if err != nil {
		return err
	}
	withdrawPayload, ok := tx.Payload.(*PayloadWithdrawAsset)
	if !ok {
		return errors.New("Invalid payload type.")
	}

	if sidechain.DbCache == nil {
		dbCache, err := sidechain.OpenDataStore()
		if err != nil {
			errors.New("Open data store failed")
		}
		sidechain.DbCache = dbCache
	}

	ok, err = sidechain.DbCache.HasSideChainTx(withdrawPayload.SideChainTransactionHash)
	if err != nil {
		return err
	}
	if ok {
		return errors.New("Reduplicate withdraw transaction.")
	}
	err = sidechain.DbCache.AddSideChainTx(withdrawPayload.SideChainTransactionHash, withdrawPayload.GenesisBlockAddress)
	if err != nil {
		return err
	}

	//hash, err := DefaultLedger.Store.GetBlockHash(uint32(withdrawPayload.BlockHeight))
	//if err != nil {
	//	return err
	//}
	//
	//block, err := DefaultLedger.Store.GetBlock(hash)
	//if err != nil {
	//	return err
	//}

	arbitrators, err := config.Parameters.GetArbitrators()
	if err != nil {
		return err
	}
	if len(arbitrators) != len(publicKeys) {
		return errors.New("Invalid arbitrator count.")
	}

	for arbitrator := range arbitrators {
		found := false
		for pk := range publicKeys {
			if arbitrator == pk {
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
