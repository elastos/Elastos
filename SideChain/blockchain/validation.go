package blockchain

import (
	"errors"
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/vm"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

func RunPrograms(tx *core.Transaction, hashes []Uint168, programs []*core.Program) error {
	if tx == nil {
		return errors.New("invalid data content nil transaction")
	}
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
		se := vm.NewExecutionEngine(core.TransactionHelper.GetDataContainer(programHash, tx),
			new(vm.CryptoECDsa), vm.MAXSTEPS, nil, nil)
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

func SortPrograms(programs []*core.Program) (err error) {
	defer func() {
		if code := recover(); code != nil {
			err = fmt.Errorf("invalid program code %x", code)
		}
	}()
	sort.Sort(byHash(programs))
	return err
}

type byHash []*core.Program

func (p byHash) Len() int      { return len(p) }
func (p byHash) Swap(i, j int) { p[i], p[j] = p[j], p[i] }
func (p byHash) Less(i, j int) bool {
	hashi, err := crypto.ToProgramHash(p[i].Code)
	if err != nil {
		panic(p[i].Code)
	}
	hashj, err := crypto.ToProgramHash(p[j].Code)
	if err != nil {
		panic(p[j].Code)
	}
	return hashi.Compare(*hashj) < 0
}
