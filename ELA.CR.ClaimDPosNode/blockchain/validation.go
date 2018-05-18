package blockchain

import (
	"bytes"
	"errors"
	"sort"

	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/sidechain"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

func VerifySignature(tx *Transaction) (bool, error) {
	hashes, err := GetTxProgramHashes(tx)
	if err != nil {
		return false, err
	}

	programs := tx.Programs
	Length := len(hashes)
	if Length != len(programs) {
		return false, errors.New("The number of data hashes is different with number of programs.")
	}

	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	data := buf.Bytes()

	for i := 0; i < len(programs); i++ {

		code := programs[i].Code
		param := programs[i].Parameter

		programHash, err := crypto.ToProgramHash(code)
		if err != nil {
			return false, err
		}

		// Get transaction type
		signType, err := crypto.GetScriptType(code)
		if err != nil {
			return false, err
		}

		if !hashes[i].IsEqual(*programHash) && signType != crypto.CROSSCHAIN {
			return false, errors.New("The data hashes is different with corresponding program code.")
		}

		if signType == crypto.STANDARD {
			// Remove length byte and sign type byte
			publicKeyBytes := code[1 : len(code)-1]

			return checkStandardSignature(publicKeyBytes, data, param)

		} else if signType == crypto.MULTISIG {
			publicKeys, err := crypto.ParseMultisigScript(code)
			if err != nil {
				return false, err
			}
			return checkMultiSignSignatures(code, param, data, publicKeys)

		} else if signType == crypto.CROSSCHAIN {
			publicKeys, err := crypto.ParseMultisigScript(code)
			if err != nil {
				return false, err
			}

			return checkMultiSignSignatures(code, param, data, publicKeys)
		} else {
			return false, errors.New("unknown signature type")
		}
	}

	return true, nil
}

func GetTxProgramHashes(tx *Transaction) ([]Uint168, error) {
	if tx == nil {
		return nil, errors.New("[Transaction],GetProgramHashes transaction is nil.")
	}
	hashes := make([]Uint168, 0)
	uniqueHashes := make([]Uint168, 0)
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
			dataHash, err := Uint168FromBytes(attribute.Data)
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
	uniq := make(map[Uint168]bool)
	for _, v := range hashes {
		uniq[v] = true
	}
	for k := range uniq {
		uniqueHashes = append(uniqueHashes, k)
	}
	sort.Sort(byProgramHashes(uniqueHashes))
	return uniqueHashes, nil
}

func checkStandardSignature(publicKeyBytes, content, signature []byte) (bool, error) {
	if len(signature) != crypto.SignatureScriptLength {
		return false, errors.New("Invalid signature length")
	}

	publicKey, err := crypto.DecodePoint(publicKeyBytes)
	if err != nil {
		return false, err
	}
	err = crypto.Verify(*publicKey, content, signature[1:])
	if err == nil {
		return false, err
	}
	return true, nil
}

func checkMultiSignSignatures(code, param, content []byte, publicKeys [][]byte) (bool, error) {
	// Get N parameter
	n := int(code[len(code)-2]) - crypto.PUSH1 + 1
	// Get M parameter
	m := int(code[0]) - crypto.PUSH1 + 1
	if m < 1 || m > n {
		return false, errors.New("invalid multi sign script code")
	}
	if len(publicKeys) != n {
		return false, errors.New("invalid multi sign public key script count")
	}

	signatureCount := 0
	for i := 0; i < len(param); i += crypto.SignatureScriptLength {
		// Remove length byte
		sign := param[i : i+crypto.SignatureScriptLength][1:]
		// Get signature index, if signature exists index will not be -1
		index := -1
		for i, publicKey := range publicKeys {
			pubKey, err := crypto.DecodePoint(publicKey[1:])
			if err != nil {
				return false, err
			}
			err = crypto.Verify(*pubKey, content, sign)
			if err == nil {
				index = i
			}
		}
		if index != -1 {
			signatureCount++
		}
	}
	// Check signature count
	if signatureCount < m {
		return false, errors.New("invalid signature count")
	}

	return true, nil
}

func checkCrossChainTransaction(txn *Transaction) error {
	if !txn.IsWithdrawTx() {
		return nil
	}

	hashes, err := GetTxProgramHashes(txn)
	if err != nil {
		return err
	}

	programs := txn.Programs
	Length := len(hashes)
	if Length != len(programs) {
		return errors.New("The number of data hashes is different with number of programs.")
	}

	buf := new(bytes.Buffer)
	txn.SerializeUnsigned(buf)
	for i := 0; i < len(programs); i++ {

		code := programs[i].Code

		// Get transaction type
		signType, err := crypto.GetScriptType(code)
		if err != nil {
			return err
		}

		if signType == crypto.CROSSCHAIN {
			publicKeys, err := crypto.ParseMultisigScript(code)
			if err != nil {
				return err
			}

			if err := checkCrossChainArbitrators(txn, publicKeys); err != nil {
				return errors.New("checkCrossChainArbitrators failed")
			}
		}
	}
	return nil
}

func checkCrossChainArbitrators(txn *Transaction, publicKeys [][]byte) error {
	withdrawPayload, ok := txn.Payload.(*PayloadWithdrawAsset)
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

	ok, err := sidechain.DbCache.HasSideChainTx(withdrawPayload.SideChainTransactionHash)
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

type byProgramHashes []Uint168

func (a byProgramHashes) Len() int      { return len(a) }
func (a byProgramHashes) Swap(i, j int) { a[i], a[j] = a[j], a[i] }
func (a byProgramHashes) Less(i, j int) bool {
	if a[i].Compare(a[j]) > 0 {
		return false
	} else {
		return true
	}
}
