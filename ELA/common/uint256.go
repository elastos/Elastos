package common

import (
	"bytes"
	"encoding/binary"
	"errors"
	"io"
)

const UINT256SIZE int = 32

type Uint256 [UINT256SIZE]uint8

func (u *Uint256) CompareTo(o *Uint256) int {
	x := u.ToArray()
	y := o.ToArray()

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

func (u *Uint256) ToArray() []byte {
	var x []byte = make([]byte, UINT256SIZE)
	for i := 0; i < 32; i++ {
		x[i] = byte(u[i])
	}

	return x
}
func (u *Uint256) ToArrayReverse() []byte {
	var x []byte = make([]byte, UINT256SIZE)
	for i, j := 0, UINT256SIZE-1; i < j; i, j = i+1, j-1 {
		x[i], x[j] = byte(u[j]), byte(u[i])
	}
	return x
}
func (u *Uint256) Serialize(w io.Writer) (int, error) {
	buf := bytes.NewBuffer([]byte{})
	binary.Write(buf, binary.LittleEndian, u)

	len, err := w.Write(buf.Bytes())

	if err != nil {
		return 0, err
	}

	return len, nil
}

func (u *Uint256) Deserialize(r io.Reader) error {
	p := make([]byte, UINT256SIZE)
	n, err := r.Read(p)

	if n <= 0 || err != nil {
		return err
	}

	buf := bytes.NewBuffer(p)
	binary.Read(buf, binary.LittleEndian, u)

	return nil
}

func (u *Uint256) String() string {
	return BytesToHexString(u.ToArray())
}

func Uint256ParseFromBytes(f []byte) (Uint256, error) {
	if len(f) != UINT256SIZE {
		return Uint256{}, errors.New("[Common]: Uint256ParseFromBytes err, len != 32")
	}

	var hash [32]uint8
	for i := 0; i < 32; i++ {
		hash[i] = f[i]
	}
	return Uint256(hash), nil
}
