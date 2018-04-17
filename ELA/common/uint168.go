package common

import (
	"io"
	"errors"
	"math/big"
	"encoding/binary"

	"github.com/itchyny/base58-go"
)

const UINT168SIZE = 21

type Uint168 [UINT168SIZE]uint8

func (u Uint168) String() string {
	return BytesToHexString(u.Bytes())
}

func (u *Uint168) Valid() bool {
	var empty = Uint168{}
	if u[0] == 18 || u[0] == 33 || u[0] == 75 || *u == empty {
		return true
	}
	return false
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

func (u Uint168) IsEqual(o Uint168) bool {
	return u == o
}

func (u *Uint168) Bytes() []byte {
	var x = make([]byte, UINT168SIZE)
	copy(x, u[:])
	return x
}

func (u *Uint168) Serialize(w io.Writer) error {
	return binary.Write(w, binary.LittleEndian, u)
}

func (u *Uint168) Deserialize(r io.Reader) error {
	return binary.Read(r, binary.LittleEndian, u)
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
		return nil, errors.New("Uint168FromBytes err, len != 21")
	}

	if bytes[0] != 18 && bytes[0] != 33 && bytes[0] != 75 {
		return nil, errors.New("invalid address type, unknown prefix")
	}

	var hash = &Uint168{}
	for i := 0; i < UINT168SIZE; i++ {
		hash[i] = bytes[i]
	}

	return hash, nil
}

func Uint168FromAddress(address string) (*Uint168, error) {
	decoded, err := base58.BitcoinEncoding.Decode([]byte(address))
	if err != nil {
		return nil, err
	}

	x, _ := new(big.Int).SetString(string(decoded), 10)

	ph, err := Uint168FromBytes(x.Bytes()[0:21])
	if err != nil {
		return nil, err
	}

	addr, err := ph.ToAddress()
	if err != nil {
		return nil, err
	}

	if addr != address {
		return nil, errors.New("[AddressToProgramHash]: decode address verify failed.")
	}

	return ph, nil
}
