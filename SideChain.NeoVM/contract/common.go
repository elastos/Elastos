package contract

import (
	"errors"
	"crypto/sha256"
	"golang.org/x/crypto/ripemd160"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	pPrefixSmartContract = 0x1c
)


func ToCodeHash(code []byte) (*common.Uint168, error) {
	if len(code) < 1 {
		return nil, errors.New("[ToProgramHash] failed, empty program code")
	}
	hash := sha256.Sum256(code)
	md160 := ripemd160.New()
	md160.Write(hash[:])
	data := md160.Sum([]byte{pPrefixSmartContract})
	return common.Uint168FromBytes(data)
}