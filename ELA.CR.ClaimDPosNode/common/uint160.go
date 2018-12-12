package common

import (
	"bytes"
	"encoding/binary"
	"errors"
	"io"
	"sort"
)

const UINT160SIZE int = 20

type Uint160 [UINT160SIZE]uint8

func (u *Uint160) Compare(o Uint160) int {
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

func (u Uint160) IsEqual(o Uint160) bool {
	return bytes.Equal(u[:], o[:])
}

func (u *Uint160) ToArray() []byte {
	var x = make([]byte, UINT160SIZE)
	for i := 0; i < 20; i++ {
		x[i] = byte(u[i])
	}

	return x
}
func (u *Uint160) ToArrayReverse() []byte {
	var x = make([]byte, UINT160SIZE)
	for i, j := 0, UINT160SIZE-1; i < j; i, j = i+1, j-1 {
		x[i], x[j] = byte(u[j]), byte(u[i])
	}
	return x
}
func (u *Uint160) Serialize(w io.Writer) (int, error) {
	b_buf := bytes.NewBuffer([]byte{})
	binary.Write(b_buf, binary.LittleEndian, u)

	len, err := w.Write(b_buf.Bytes())

	if err != nil {
		return 0, err
	}

	return len, nil
}

func (u *Uint160) Deserialize(r io.Reader) error {
	p := make([]byte, UINT160SIZE)
	n, err := r.Read(p)

	if n <= 0 || err != nil {
		return err
	}

	b_buf := bytes.NewBuffer(p)
	binary.Read(b_buf, binary.LittleEndian, u)

	return nil
}

func Uint160ParseFromBytes(f []byte) (Uint160, error) {
	if len(f) != UINT160SIZE {
		return Uint160{}, errors.New("[Common]: Uint160ParseFromBytes err, len != 20")
	}

	var hash [20]uint8
	for i := 0; i < 20; i++ {
		hash[i] = f[i]
	}
	return Uint160(hash), nil
}

func Uint160ParseFromUint168(u168 Uint168) Uint160 {
	buf := u168[1:21]
	var hash [20]uint8
	for i := 0; i < 20; i++ {
		hash[i] = buf[i]
	}
	return Uint160(hash)
}

func SortCodeHashes(hashes []Uint160) {
	sort.Sort(codeHashes(hashes))
}

type codeHashes []Uint160

func (a codeHashes) Len() int           { return len(a) }
func (a codeHashes) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a codeHashes) Less(i, j int) bool { return a[i].Compare(a[j]) < 0 }

func SortProgramHashByCodeHash(hashes []Uint168) {
	sort.Sort(programHashes(hashes))
}

type programHashes []Uint168

func (a programHashes) Len() int      { return len(a) }
func (a programHashes) Swap(i, j int) { a[i], a[j] = a[j], a[i] }
func (a programHashes) Less(i, j int) bool {
	hashI := Uint160ParseFromUint168(a[i])
	hashJ := Uint160ParseFromUint168(a[i])

	return hashI.Compare(hashJ) < 0
}
