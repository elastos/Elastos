package common

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"io"
	"math/big"

	"github.com/itchyny/base58-go"
)

//This is a magical change of Uint168, with length from 20 to 21.
//In fact, It is UINT168 now.
const UINT168SIZE = 21

type Uint168 [UINT168SIZE]uint8

func (u *Uint168) CompareTo(o Uint168) int {
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

func (u *Uint168) ToArray() []byte {
	var x = make([]byte, UINT168SIZE)
	for i := 0; i < UINT168SIZE; i++ {
		x[i] = byte(u[i])
	}

	return x
}
func (u *Uint168) ToArrayReverse() []byte {
	var x = make([]byte, UINT168SIZE)
	for i, j := 0, UINT168SIZE-1; i < j; i, j = i+1, j-1 {
		x[i], x[j] = byte(u[j]), byte(u[i])
	}
	return x
}
func (u *Uint168) Serialize(w io.Writer) (int, error) {
	b_buf := bytes.NewBuffer([]byte{})
	binary.Write(b_buf, binary.LittleEndian, u)

	len, err := w.Write(b_buf.Bytes())

	if err != nil {
		return 0, err
	}

	return len, nil
}

func (f *Uint168) Deserialize(r io.Reader) error {
	p := make([]byte, UINT168SIZE)
	n, err := r.Read(p)

	if n <= 0 || err != nil {
		return err
	}

	b_buf := bytes.NewBuffer(p)
	binary.Read(b_buf, binary.LittleEndian, f)

	return nil
}

func (f *Uint168) ToAddress() (string, error) {
	data := f.ToArray()
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

func Uint168FromBytes(f []byte) (Uint168, error) {

	if len(f) != UINT168SIZE {
		return Uint168{}, errors.New("[Common]: Uint168ParseFromBytes err, len != 21")
	}

	var hash [UINT168SIZE]uint8
	for i := 0; i < UINT168SIZE; i++ {
		hash[i] = f[i]
	}
	return Uint168(hash), nil
}

func Uint68FromAddress(address string) (Uint168, error) {
	encoding := base58.BitcoinEncoding

	decoded, err := encoding.Decode([]byte(address))
	if err != nil {
		return Uint168{}, err
	}

	x, _ := new(big.Int).SetString(string(decoded), 10)

	ph, err := Uint168FromBytes(x.Bytes()[0:21])
	if err != nil {
		return Uint168{}, err
	}

	addr, err := ph.ToAddress()
	if err != nil {
		return Uint168{}, err
	}

	if addr != address {
		return Uint168{}, errors.New("[AddressToProgramHash]: decode address verify failed.")
	}

	return ph, nil
}
