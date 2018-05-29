package blockchain

import (
	"bytes"
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/mainchain"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/vm"
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

func VerifySignature(tx *core.Transaction) error {
	if tx.TxType == core.RechargeToSideChain {
		if err := spv.VerifyTransaction(tx); err != nil {
			return err
		}
		return nil
	}

	hashes, err := GetTxProgramHashes(tx)
	if err != nil {
		return err
	}

	return RunPrograms(tx, hashes, tx.Programs)
}

func RunPrograms(data interfaces.IDataContainer, hashes []Uint168, programs []*core.Program) error {
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
		se := vm.NewExecutionEngine(data, new(vm.CryptoECDsa), vm.MAXSTEPS, nil, nil)
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

	//remove dupilicated hashes
	uniq := make(map[Uint168]bool)
	for _, v := range hashes {
		uniq[v] = true
	}
	for k := range uniq {
		uniqueHashes = append(uniqueHashes, k)
	}
	SortProgramHashes(uniqueHashes)
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
