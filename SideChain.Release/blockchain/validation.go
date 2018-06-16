package blockchain

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"sort"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/mainchain"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/vm"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	. "github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
	"golang.org/x/crypto/ripemd160"
)

func VerifySignature(tx *core.Transaction) error {
	if tx.IsRechargeToSideChainTx() {
		if err := spv.VerifyTransaction(tx); err != nil {
			return err
		}
		return nil
	}

	hashes, err := GetTxProgramHashes(tx)
	if err != nil {
		return err
	}

	// Add ID program hash to hashes
	if tx.IsRegisterIdentificationTx() {
		for _, output := range tx.Outputs {
			if output.ProgramHash[0] == PrefixRegisterId {
				hashes = append(hashes, output.ProgramHash)
				break
			}
		}
	}

	// Sort first
	SortProgramHashes(hashes)
	SortPrograms(tx.Programs)

	return RunPrograms(tx, hashes, tx.Programs)
}

func RunPrograms(tx *core.Transaction, hashes []Uint168, programs []*core.Program) error {
	if len(hashes) != len(programs) {
		return errors.New("The number of data hashes is different with number of programs.")
	}

	for i := 0; i < len(programs); i++ {
		programHash, err := crypto.ToProgramHash(programs[i].Code)
		if err != nil {
			return err
		}

		if !hashes[i].IsEqual(*programHash) {
			return errors.New("The data hashes is different with corresponding program code.")
		}
		//execute program on VM
		se := vm.NewExecutionEngine(tx.GetDataContainer(programHash), new(vm.CryptoECDsa), vm.MAXSTEPS, nil, nil)
		se.LoadScript(programs[i].Code, false)
		se.LoadScript(programs[i].Parameter, true)
		se.Execute()

		if se.GetState() != vm.HALT {
			return errors.New("[VM] Finish State not equal to HALT.")
		}

		if se.GetEvaluationStack().Count() != 1 {
			return errors.New("[VM] Execute Engine Stack Count Error.")
		}

		success := se.GetExecuteResult()
		if !success {
			return errors.New("[VM] Check Sig FALSE.")
		}
	}

	return nil
}

func GetTxProgramHashes(tx *core.Transaction) ([]Uint168, error) {
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
		if attribute.Usage == core.Script {
			dataHash, err := Uint168FromBytes(attribute.Data)
			if err != nil {
				return nil, errors.New("[Transaction], GetProgramHashes err.")
			}
			hashes = append(hashes, *dataHash)
		}
	}

	//remove duplicated hashes
	uniq := make(map[Uint168]bool)
	for _, v := range hashes {
		uniq[v] = true
	}
	for k := range uniq {
		uniqueHashes = append(uniqueHashes, k)
	}
	return uniqueHashes, nil
}

func checkCrossChainTransaction(txn *core.Transaction) error {
	if !txn.IsRechargeToSideChainTx() {
		return nil
	}

	depositPayload, ok := txn.Payload.(*core.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("Invalid payload type.")
	}

	if mainchain.DbCache == nil {
		dbCache, err := mainchain.OpenDataStore()
		if err != nil {
			errors.New("Open data store failed")
		}
		mainchain.DbCache = dbCache
	}

	mainChainTransaction := new(core.Transaction)
	reader := bytes.NewReader(depositPayload.MainChainTransaction)
	if err := mainChainTransaction.Deserialize(reader); err != nil {
		return errors.New("PayloadRechargeToSideChain mainChainTransaction deserialize failed")
	}

	ok, err := mainchain.DbCache.HasMainChainTx(mainChainTransaction.Hash().String())
	if err != nil {
		return err
	}
	if ok {
		return errors.New("Reduplicate withdraw transaction.")
	}
	err = mainchain.DbCache.AddMainChainTx(mainChainTransaction.Hash().String())
	if err != nil {
		return err
	}
	return nil
}

func CheckRechargeToSideChainTransaction(txn *core.Transaction) error {
	proof := new(MerkleProof)
	mainChainTransaction := new(ela.Transaction)

	payloadRecharge, ok := txn.Payload.(*core.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("Invalid payload core.PayloadRechargeToSideChain")
	}

	reader := bytes.NewReader(payloadRecharge.MerkleProof)
	if err := proof.Deserialize(reader); err != nil {
		return errors.New("RechargeToSideChain payload deserialize failed")
	}
	reader = bytes.NewReader(payloadRecharge.MainChainTransaction)
	if err := mainChainTransaction.Deserialize(reader); err != nil {
		return errors.New("RechargeToSideChain mainChainTransaction deserialize failed")
	}

	payloadObj, ok := mainChainTransaction.Payload.(*ela.PayloadTransferCrossChainAsset)
	if !ok {
		return errors.New("Invalid payload ela.PayloadTransferCrossChainAsset")
	}

	genesisHash, _ := DefaultLedger.Store.GetBlockHash(uint32(0))
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(genesisHash.Bytes())))
	buf.Write(genesisHash.Bytes())
	buf.WriteByte(byte(CROSSCHAIN))

	sum168 := func(prefix byte, code []byte) []byte {
		hash := sha256.Sum256(code)
		md160 := ripemd160.New()
		md160.Write(hash[:])
		return md160.Sum([]byte{prefix})
	}

	genesisProgramHash, err := Uint168FromBytes(sum168(PrefixCrossChain, buf.Bytes()))
	if err != nil {
		return errors.New("genesis block bytes to program hash failed")
	}

	//check output fee and rate
	rate := Fixed64(config.Parameters.ExchangeRate)

	var oriTotalFee Fixed64
	var oriOutputTotalAmount Fixed64
	for i := 0; i < len(mainChainTransaction.Outputs); i++ {
		if mainChainTransaction.Outputs[i].ProgramHash.IsEqual(*genesisProgramHash) {
			oriOutputTotalAmount += mainChainTransaction.Outputs[i].Value
			oriTotalFee += mainChainTransaction.Outputs[i].Value - payloadObj.CrossChainAmount[i]

			programHash, err := Uint168FromAddress(payloadObj.CrossChainAddress[i])
			if err != nil {
				return errors.New("Check deposit transaction failed, invalid payload cross chain address")
			}
			isContained := false
			for _, output := range txn.Outputs {
				if output.ProgramHash == *programHash && output.Value == payloadObj.CrossChainAmount[i]*rate {
					isContained = true
					break
				}
			}
			if !isContained {
				return errors.New("Check deposit transaction failed, invalid outputs")
			}
		}
	}

	var targetOutputTotalAmount Fixed64
	for _, output := range txn.Outputs {
		targetOutputTotalAmount += output.Value
	}

	if targetOutputTotalAmount != (oriOutputTotalAmount-oriTotalFee)*rate {
		return errors.New("Check deposit transaction failed, output and fee verify failed")
	}

	return nil
}

func SortPrograms(programs []*core.Program) {
	sort.Sort(byHash(programs))
}

type byHash []*core.Program

func (p byHash) Len() int      { return len(p) }
func (p byHash) Swap(i, j int) { p[i], p[j] = p[j], p[i] }
func (p byHash) Less(i, j int) bool {
	hashi, _ := crypto.ToProgramHash(p[i].Code)
	hashj, _ := crypto.ToProgramHash(p[j].Code)
	return hashi.Compare(*hashj) < 0
}
