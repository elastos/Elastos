package common

import (
	"io"
	"bytes"
	"errors"
	"math/big"
	"crypto/sha256"
	"encoding/binary"

	"github.com/itchyny/base58-go"
)

const UINT168SIZE = 21

type Uint168 [UINT168SIZE]uint8

func (self Uint168) String() string {
	return BytesToHexString(self.ToArray())
}

func (self *Uint168) CompareTo(other *Uint168) int {
	x := self.ToArray()
	y := other.ToArray()

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

func (self *Uint168) ToArray() []byte {
	var x = make([]byte, UINT168SIZE)
	for i := 0; i < UINT168SIZE; i++ {
		x[i] = byte(self[i])
	}

	return x
}

func (self *Uint168) ToArrayReverse() []byte {
	var x = make([]byte, UINT168SIZE)
	for i, j := 0, UINT168SIZE-1; i < j; i, j = i+1, j-1 {
		x[i], x[j] = byte(self[j]), byte(self[i])
	}
	return x
}

func (self *Uint168) Serialize(w io.Writer) (int, error) {
	b_buf := bytes.NewBuffer([]byte{})
	binary.Write(b_buf, binary.LittleEndian, self)

	len, err := w.Write(b_buf.Bytes())

	if err != nil {
		return 0, err
	}

	return len, nil
}

func (self *Uint168) Deserialize(r io.Reader) error {
	p := make([]byte, UINT168SIZE)
	n, err := r.Read(p)

	if n <= 0 || err != nil {
		return err
	}

	b_buf := bytes.NewBuffer(p)
	binary.Read(b_buf, binary.LittleEndian, self)

	return nil
}

func (self *Uint168) ToAddress() (string, error) {
	data := self.ToArray()
	temp := sha256.Sum256(data)
	temps := sha256.Sum256(temp[:])
	data = append(data, temps[0:4]...)

	bi := new(big.Int).SetBytes(data).String()
	encoding := base58.BitcoinEncoding
	encoded, err := encoding.Encode([]byte(bi))
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
	encoding := base58.BitcoinEncoding

	decoded, err := encoding.Decode([]byte(address))
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
