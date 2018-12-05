package params

import (
	"errors"
	"crypto/sha256"
	"bytes"
	"encoding/binary"

	"golang.org/x/crypto/ripemd160"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
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
	data := md160.Sum([]byte{})
	data = common.BytesReverse(data)
	bytes := []byte{PrefixSmartContract}
	bytes = append(bytes, data...)
	return common.Uint168FromBytes(bytes)
}

func UInt168ToUInt160(hash *common.Uint168) []byte {
	hashBytes := make([]byte, len(hash) - 1)
	data := hash.Bytes()
	copy(hashBytes, data[1 : len(hash)])
	return hashBytes
}

func ToProgramHash(code []byte) (*common.Uint168, error){
	hash, err := crypto.ToProgramHash(code)
	if hash == nil && len(code) > 0 {
		hash, err = ToCodeHash(code)
	}
	return hash, err
}

func IntToBytes(n int64) []byte {
	buffer := bytes.NewBuffer([]byte{})
	binary.Write(buffer, binary.BigEndian, n)
	return buffer.Bytes()
}

func BytesToInt(b []byte) int64 {
	buffer := bytes.NewBuffer(b)
	var n int64
	binary.Read(buffer, binary.BigEndian, &n)
	return n
}