package params

import (
	"errors"
	"crypto/sha256"
	"golang.org/x/crypto/ripemd160"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	PrefixSmartContract = 0x1c
)


func ToCodeHash(code []byte) (*common.Uint168, error) {
	if len(code) < 1 {
		return nil, errors.New("[ToProgramHash] failed, empty program code")
	}
	hash := sha256.Sum256(code)
	md160 := ripemd160.New()
	md160.Write(hash[:])
	data := md160.Sum([]byte{PrefixSmartContract})
	return common.Uint168FromBytes(data)
}

func UInt168ToUInt160(hash *common.Uint168) []byte {
	hashBytes := make([]byte, len(hash) - 1)
	data := hash.Bytes()
	copy(hashBytes, data[1 : len(hash)])
	return hashBytes
}