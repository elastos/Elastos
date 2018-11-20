package avm

import (
	"math"
	"errors"
)

func opCheckSequenceVerify(e *ExecutionEngine) (VMState, error) {
	if e.dataContainer == nil {
		return FAULT, nil
	}
	references, err := e.table.GetTxReference(&e.dataContainer)
	if err != nil {
		return FAULT, err
	}
	for input, output := range references {
		if output.GetOutputLock() == 0 {
			continue
		}

		if input.GetSequence() != math.MaxUint32-1 {
			return FAULT, errors.New("Invalid input sequence")
		}
	}
	return NONE, nil
}

func opCheckLockTimeVerify(e *ExecutionEngine) (VMState, error) {
	//todo completed this
	//if e.dataContainer == nil {
	//	return FAULT, nil
	//}
	//txn := e.GetDataContainer().(interfaces.IUtxolock)
	//lockTime := PopBigInt(e)
	//if txn.GetLockTime() < uint32(lockTime.Uint64()) {
	//	return FAULT, errors.New("UTXO output locked")
	//}
	return NONE, nil
}