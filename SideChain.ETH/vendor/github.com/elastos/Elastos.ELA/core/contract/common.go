package contract

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/vm"
)

type ContractType byte

const (
	Signature ContractType = iota
	MultiSig
	Custom
)

// Extension OP_CODE
const (
	CROSSCHAIN = 0xAF
)

func IsStandard(code []byte) bool {
	if len(code) != 35 {
		return false
	}
	if code[0] != 33 || code[34] != byte(vm.CHECKSIG) {
		return false
	}
	return true
}

func IsMultiSig(code []byte) bool {
	var m int16 = 0
	var n int16 = 0
	i := 0

	if len(code) < 37 {
		return false
	}
	if code[i] > byte(vm.PUSH16) {
		return false
	}
	if code[i] < byte(vm.PUSH1) && code[i] != 1 && code[i] != 2 {
		return false
	}

	switch code[i] {
	case 1:
		i++
		m = int16(code[i])
		i++
		break
	case 2:
		i++
		m = common.BytesToInt16(code[i:])
		i += 2
		break
	default:
		m = int16(code[i]) - 80
		i++
		break
	}

	if m < 1 || m > 1024 {
		return false
	}

	for code[i] == 33 {
		i += 34
		if len(code) <= i {
			return false
		}
		n++
	}
	if n < m || n > 1024 {
		return false
	}

	switch code[i] {
	case 1:
		i++
		if n != int16(code[i]) {
			return false
		}
		i++
		break
	case 2:
		i++
		if n != common.BytesToInt16(code[i:]) {
			return false
		}
		i += 2
		break
	default:
		if n != (int16(code[i]) - 80) {
			return false
		}
		i++
		break
	}

	if code[i] != byte(vm.CHECKMULTISIG) {
		return false
	}
	i++
	if len(code) != i {
		return false
	}

	return true
}

func GetCodeType(code []byte) ContractType {
	if IsStandard(code) {
		return Signature
	}
	if IsMultiSig(code) {
		return MultiSig
	}
	return Custom
}

func GetPrefixType(programHash common.Uint168) PrefixType {
	prefixType := PrefixType(programHash[0])
	return PrefixType(prefixType)
}
