package common

import (
	"crypto/sha256"
	"errors"
	"encoding/hex"
	"os"
)

func BytesToHexString(data []byte) string {
	return hex.EncodeToString(data)
}

func HexStringToBytes(value string) ([]byte, error) {
	return hex.DecodeString(value)
}

func BytesReverse(u []byte) []byte {
	for i, j := 0, len(u)-1; i < j; i, j = i+1, j-1 {
		u[i], u[j] = u[j], u[i]
	}
	return u
}

func HexStringToBytesReverse(value string) ([]byte, error) {
	u, err := hex.DecodeString(value)
	if err != nil {
		return u, err
	}
	return BytesReverse(u), err
}

func CompareHeight(blockHeight uint64, heights []uint64) bool {
	for _, height := range heights {
		if blockHeight < height {
			return false
		}
	}
	return true
}

func GetUint16Array(source []byte) ([]uint16, error) {
	if source == nil {
		return nil, errors.New("[Common] , GetUint16Array err, source = nil")
	}

	if len(source)%2 != 0 {
		return nil, errors.New("[Common] , GetUint16Array err, length of source is odd.")
	}

	dst := make([]uint16, len(source)/2)
	for i := 0; i < len(source)/2; i++ {
		dst[i] = uint16(source[i*2]) + uint16(source[i*2+1])*256
	}

	return dst, nil
}

func ToByteArray(source []uint16) []byte {
	dst := make([]byte, len(source)*2)
	for i := 0; i < len(source); i++ {
		dst[i*2] = byte(source[i] % 256)
		dst[i*2+1] = byte(source[i] / 256)
	}

	return dst
}

func ClearBytes(arr []byte) {
	for i := 0; i < len(arr); i++ {
		arr[i] = 0
	}
}

func FileExisted(filename string) bool {
	_, err := os.Stat(filename)
	return err == nil || os.IsExist(err)
}

func Sha256D(data []byte) [32]byte {
	once := sha256.Sum256(data)
	return sha256.Sum256(once[:])
}
