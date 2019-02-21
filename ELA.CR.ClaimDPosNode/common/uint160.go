package common

import (
	"bytes"
	"errors"
	"io"
	"sort"
)

const UINT160SIZE int = 20

type Uint160 [UINT160SIZE]uint8

func (u Uint160) Compare(o Uint160) int {
	for i := UINT160SIZE - 1; i >= 0; i-- {
		if u[i] > o[i] {
			return 1
		}
		if u[i] < o[i] {
			return -1
		}
	}
	return 0
}

func (u Uint160) IsEqual(o Uint160) bool {
	return bytes.Equal(u[:], o[:])
}

func (u Uint160) Bytes() []byte {
	return u[:]
}

func (u *Uint160) Serialize(w io.Writer) error {
	_, err := w.Write(u[:])
	return err
}

func (u *Uint160) Deserialize(r io.Reader) error {
	_, err := io.ReadFull(r, u[:])
	return err
}

func Uint160FromBytes(bytes []byte) (Uint160, error) {
	if len(bytes) != UINT160SIZE {
		return Uint160{}, errors.New("[Common]: Uint160FromBytes err, len != 20")
	}

	var hash Uint160
	copy(hash[:], bytes)
	return Uint160(hash), nil
}

func SortUint160(hashes []Uint160) {
	sort.Slice(hashes, func(i, j int) bool {
		return hashes[i].Compare(hashes[j]) < 0
	})
}
