package common

import (
	"errors"
	"io"
	"math/big"
	"sort"

	"github.com/itchyny/base58-go"
)

const (
	UINT168SIZE = 21
	// Address types
	STANDARD   = 0xAC
	REGISTERID = 0xAD
	MULTISIG   = 0xAE
	CROSSCHAIN = 0xAF

	PrefixStandard   = 0x21
	PrefixRegisterId = 0x67
	PrefixMultisig   = 0x12
	PrefixCrossChain = 0x4B
)

type Uint168 [UINT168SIZE]uint8

func (u Uint168) String() string {
	return BytesToHexString(u.Bytes())
}

func (u Uint168) Compare(o Uint168) int {
	for i := UINT168SIZE - 1; i >= 0; i-- {
		if u[i] > o[i] {
			return 1
		}
		if u[i] < o[i] {
			return -1
		}
	}
	return 0
}

func (u *Uint168) IsEqual(o Uint168) bool {
	return *u == o
}

func (u *Uint168) Bytes() []byte {
	var x = make([]byte, UINT168SIZE)
	copy(x, u[:])
	return x
}

func (u *Uint168) Serialize(w io.Writer) error {
	_, err := w.Write(u[:])
	return err
}

func (u *Uint168) Deserialize(r io.Reader) error {
	_, err := io.ReadFull(r, u[:])
	return err
}

func (u *Uint168) ToAddress() (string, error) {
	data := u.Bytes()
	checksum := Sha256D(data)
	data = append(data, checksum[0:4]...)

	bi := new(big.Int).SetBytes(data).String()
	encoded, err := base58.BitcoinEncoding.Encode([]byte(bi))
	if err != nil {
		return "", err
	}
	return string(encoded), nil
}

func Uint168FromBytes(bytes []byte) (*Uint168, error) {
	if len(bytes) != UINT168SIZE {
		return nil, errors.New("[Uint168FromBytes] error, len != 21")
	}

	var hash = &Uint168{}
	copy(hash[:], bytes)

	return hash, nil
}

func Uint168FromAddress(address string) (*Uint168, error) {
	if len(address) != 34 {
		return nil, errors.New("[Uint168FromAddress] error, len != 34")
	}

	decoded, err := base58.BitcoinEncoding.Decode([]byte(address))
	if err != nil {
		return nil, err
	}

	x, _ := new(big.Int).SetString(string(decoded), 10)

	programHash, err := Uint168FromBytes(x.Bytes()[0:21])
	if err != nil {
		return nil, err
	}

	addr, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	if addr != address {
		return nil, errors.New("[Uint168FromAddress]: decode address verify failed.")
	}

	return programHash, nil
}

func SortProgramHashes(hashes []Uint168) {
	sort.Sort(programHashes(hashes))
}

type programHashes []Uint168

func (a programHashes) Len() int           { return len(a) }
func (a programHashes) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a programHashes) Less(i, j int) bool { return a[i].Compare(a[j]) < 0 }
