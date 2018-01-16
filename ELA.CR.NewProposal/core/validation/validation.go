package validation

import (
	"errors"

	"Elastos.ELA/vm"
	. "Elastos.ELA/common"
	"Elastos.ELA/vm/interfaces"
	sig "Elastos.ELA/core/signature"
)

func VerifySignableData(signableData sig.SignableData) (bool, error) {

	hashes, err := signableData.GetProgramHashes()
	if err != nil {
		return false, err
	}

	programs := signableData.GetPrograms()
	Length := len(hashes)
	if Length != len(programs) {
		return false, errors.New("The number of data hashes is different with number of programs.")
	}

	programs = signableData.GetPrograms()

	for i := 0; i < len(programs); i++ {

		//TODO:暂时先按此方式验证，以后改为按Code的结构确定是那种脚本以及对应的前缀
		var temp Uint168
		if hashes[i][0] == 18 {
			temp, _ = ToCodeHash(programs[i].Code, 2)
		} else if hashes[i][0] == 33 {
			temp, _ = ToCodeHash(programs[i].Code, 1)
		} else {
			return false, errors.New("invalid address prefix")
		}

		if hashes[i] != temp {
			return false, errors.New("The data hashes is different with corresponding program code.")
		}
		//execute program on VM
		var cryptos interfaces.ICrypto
		cryptos = new(vm.ECDsaCrypto)
		se := vm.NewExecutionEngine(signableData, cryptos, 1200, nil, nil)
		se.LoadScript(programs[i].Code, false)
		se.LoadScript(programs[i].Parameter, true)
		se.Execute()

		if se.GetState() != vm.HALT {
			return false, errors.New("[VM] Finish State not equal to HALT.")
		}

		if se.GetEvaluationStack().Count() != 1 {
			return false, errors.New("[VM] Execute Engine Stack Count Error.")
		}

		flag := se.GetExecuteResult()
		if !flag {
			return false, errors.New("[VM] Check Sig FALSE.")
		}
	}

	return true, nil
}
