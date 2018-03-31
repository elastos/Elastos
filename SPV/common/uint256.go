package common

import (
	"io"
	"errors"
	"encoding/binary"
)

const UINT256SIZE = 32

type Uint256 [UINT256SIZE]uint8

func (u *Uint256) CompareTo(o *Uint256) int {
	x := u.Bytes()
	y := o.Bytes()

	for i := len(x) - 1; i >= 0; i-- {
		if x[i] > y[i] {
			return 1
		}
		if x[i] < y[i] {
			return -1
		}
	}

	return 0
}

func (u *Uint256) IsEqual(o *Uint256) bool {
	return u.CompareTo(o) == 0
}

func (u *Uint256) Bytes() []byte {
	var x = make([]byte, UINT256SIZE)
	for i := 0; i < 32; i++ {
		x[i] = byte(u[i])
	}

	return x
}

func (u *Uint256) BytesReverse() []byte {
	var x = make([]byte, UINT256SIZE)
	for i, j := 0, UINT256SIZE-1; i < j; i, j = i+1, j-1 {
		x[i], x[j] = byte(u[j]), byte(u[i])
	}
	return x
}

func (u *Uint256) Serialize(w io.Writer) error {
	return binary.Write(w, binary.LittleEndian, u)
}

func (u *Uint256) Deserialize(r io.Reader) error {
	return binary.Read(r, binary.LittleEndian, u)
}

func (u *Uint256) String() string {
	return BytesToHexString(u.Bytes())
}

func Uint256FromBytes(f []byte) (*Uint256, error) {
	if len(f) != UINT256SIZE {
		return nil, errors.New("[Common]: Uint256ParseFromBytes err, len != 32")
	}

	var hash Uint256
	copy(hash[:], f)

	return &hash, nil
}
