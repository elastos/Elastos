package vm

import (
	"crypto/sha1"
	"crypto/sha256"
	"errors"
	"hash"
)

func opHash(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 1 {
		return FAULT, nil
	}
	x := AssertStackItem(e.evaluationStack.Pop()).GetByteArray()
	err := pushData(e, Hash(x, e))
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opCheckSig(e *ExecutionEngine) (VMState, error) {
	if e.dataContainer == nil {
		return FAULT, nil
	}
	if e.evaluationStack.Count() < 2 {
		return FAULT, nil
	}
	pubkey := AssertStackItem(e.evaluationStack.Pop()).GetByteArray()
	signature := AssertStackItem(e.evaluationStack.Pop()).GetByteArray()
	err := e.crypto.VerifySignature(e.dataContainer.GetData(), signature, pubkey)
	err = pushData(e, err == nil)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func opCheckMultiSig(e *ExecutionEngine) (VMState, error) {
	if e.evaluationStack.Count() < 4 {
		return FAULT, errors.New("element count is not enough")
	}
	n := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if n < 1 {
		return FAULT, errors.New("invalid n in multisig")
	}
	if e.evaluationStack.Count() < n+2 {
		return FAULT, errors.New("invalid element count")
	}
	e.opCount += n
	if e.opCount > e.maxSteps {
		return FAULT, errors.New("too many OP code")
	}

	pubkeys := make([][]byte, n)
	for i := 0; i < n; i++ {
		pubkeys[i] = AssertStackItem(e.evaluationStack.Pop()).GetByteArray()
	}

	m := int(AssertStackItem(e.evaluationStack.Pop()).GetBigInteger().Int64())
	if m < 1 || m > n {
		return FAULT, errors.New("invalid m in multisig")
	}
	if e.evaluationStack.Count() < m {
		return FAULT, errors.New("signatures in stack is not enough")
	}
	if e.evaluationStack.Count() > n {
		return FAULT, errors.New("too many signatures in stack")
	}

	signatures := make([][]byte, 0, n)
	for e.evaluationStack.Count() > 0 {
		signatures = append(signatures, AssertStackItem(e.evaluationStack.Pop()).GetByteArray())
	}

	data := e.dataContainer.GetData()
	fSuccess := true
	verified := 0
	for _, sig := range signatures {
		index := -1
		for i, pubkey := range pubkeys {
			err := e.crypto.VerifySignature(data, sig, pubkey)
			if err == nil {
				index = i
				verified++
				break
			}
		}
		if index != -1 {
			pubkeys = append(pubkeys[:index], pubkeys[index+1:]...)
		} else {
			fSuccess = false
			break
		}
	}
	if verified < m {
		fSuccess = false
	}
	err := pushData(e, fSuccess)
	if err != nil {
		return FAULT, err
	}
	return NONE, nil
}

func Hash(b []byte, e *ExecutionEngine) []byte {
	var sh hash.Hash
	var bt []byte
	switch e.opCode {
	case SHA1:
		sh = sha1.New()
		sh.Write(b)
		bt = sh.Sum(nil)
	case SHA256:
		sh = sha256.New()
		sh.Write(b)
		bt = sh.Sum(nil)
	case HASH160:
		bt = e.crypto.Hash168(b)
	case HASH256:
		bt = e.crypto.Hash256(b)
	}
	return bt
}
