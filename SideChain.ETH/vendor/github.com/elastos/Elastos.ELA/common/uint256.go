package common

import (
	"bytes"
	"encoding/hex"
	"errors"
	"io"
)

const UINT256SIZE = 32

type Uint256 [UINT256SIZE]uint8

var EmptyHash = Uint256{}

func (u Uint256) Compare(o Uint256) int {
	for i := UINT256SIZE - 1; i >= 0; i-- {
		if u[i] > o[i] {
			return 1
		}
		if u[i] < o[i] {
			return -1
		}
	}
	return 0
}

func (u Uint256) IsEqual(o Uint256) bool {
	return bytes.Equal(u[:], o[:])
}

func (u Uint256) String() string {
	return BytesToHexString(u.Bytes())
}

func (u Uint256) Bytes() []byte {
	var x = make([]byte, UINT256SIZE)
	copy(x, u[:])
	return x
}

func (u *Uint256) Serialize(w io.Writer) error {
	_, err := w.Write(u[:])
	return err
}

func (u *Uint256) Deserialize(r io.Reader) error {
	_, err := io.ReadFull(r, u[:])
	return err
}

func Uint256FromBytes(f []byte) (*Uint256, error) {
	if len(f) != UINT256SIZE {
		return nil, errors.New("[Common]: Uint256ParseFromBytes err, len != 32")
	}

	var hash Uint256
	copy(hash[:], f)

	return &hash, nil
}

func Uint256FromHexString(hexHash string) (*Uint256, error) {
	if len(hexHash) != UINT256SIZE*2 {
		return nil, errors.New("[Common]: Uint256ParseFromString err, len != 64")
	}
	hashByte, err := hex.DecodeString(hexHash)
	if err != nil {
		return nil, err
	}

	var hash Uint256
	copy(hash[:], hashByte)

	return &hash, nil
}
