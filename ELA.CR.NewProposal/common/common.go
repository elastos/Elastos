package common

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"runtime"
	"sort"
	"strings"

	"golang.org/x/crypto/ripemd160"
)

func ToCodeHash(code []byte) *Uint160 {
	hash := sha256.Sum256(code)
	md160 := ripemd160.New()
	md160.Write(hash[:])
	sum := Uint160{}
	copy(sum[:], md160.Sum(nil))
	return &sum
}

func ToProgramHash(prefix byte, code []byte) *Uint168 {
	hash := sha256.Sum256(code)
	md160 := ripemd160.New()
	md160.Write(hash[:])
	sum := Uint168{}
	copy(sum[:], md160.Sum([]byte{prefix}))
	return &sum
}

func SortProgramHashByCodeHash(hashes []Uint168) {
	sort.Slice(hashes, func(i, j int) bool {
		return hashes[i].ToCodeHash().Compare(hashes[j].ToCodeHash()) < 0
	})
}

func BytesReverse(u []byte) []byte {
	for i, j := 0, len(u)-1; i < j; i, j = i+1, j-1 {
		u[i], u[j] = u[j], u[i]
	}
	return u
}

func BytesToHexString(data []byte) string {
	return hex.EncodeToString(data)
}

func HexStringToBytes(value string) ([]byte, error) {
	return hex.DecodeString(value)
}

func IntToBytes(n int) []byte {
	tmp := int32(n)
	bytesBuffer := bytes.NewBuffer([]byte{})
	binary.Write(bytesBuffer, binary.LittleEndian, tmp)
	return bytesBuffer.Bytes()
}

func BytesToInt16(b []byte) int16 {
	bytesBuffer := bytes.NewBuffer(b)
	var tmp int16
	binary.Read(bytesBuffer, binary.BigEndian, &tmp)
	return int16(tmp)
}

func ClearBytes(arr []byte) {
	for i := 0; i < len(arr); i++ {
		arr[i] = 0
	}
}

func Sha256D(data []byte) [32]byte {
	once := sha256.Sum256(data)
	return sha256.Sum256(once[:])
}

// Goid returns the current goroutine id.
func Goid() string {
	var buf [18]byte
	n := runtime.Stack(buf[:], false)
	fields := strings.Fields(string(buf[:n]))
	if len(fields) <= 1 {
		return ""
	}
	return fields[1]
}
