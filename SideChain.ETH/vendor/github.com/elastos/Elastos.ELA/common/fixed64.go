package common

import (
	"bytes"
	"encoding/binary"
	"errors"
	"io"
	"strconv"
	"strings"
)

//the 64 bit fixed-point number, precise 10^-8
type Fixed64 int64

func (f *Fixed64) Serialize(w io.Writer) error {
	err := binary.Write(w, binary.LittleEndian, int64(*f))
	if err != nil {
		return err
	}

	return nil
}

func (f *Fixed64) Deserialize(r io.Reader) error {
	p := make([]byte, 8)
	n, err := r.Read(p)
	if n <= 0 || err != nil {
		return err
	}
	buf := bytes.NewBuffer(p)
	var x int64
	err = binary.Read(buf, binary.LittleEndian, &x)
	if err != nil {
		return err
	}
	*f = Fixed64(x)
	return nil
}

func (f Fixed64) IntValue() int64 {
	return int64(f)
}

func (f Fixed64) String() string {
	var buff bytes.Buffer
	value := uint64(f)
	if f < 0 {
		buff.WriteRune('-')
		value = uint64(-f)
	}
	buff.WriteString(strconv.FormatUint(value/100000000, 10))
	value %= 100000000
	if value > 0 {
		buff.WriteRune('.')
		s := strconv.FormatUint(value, 10)
		for i := len(s); i < 8; i++ {
			buff.WriteRune('0')
		}
		buff.WriteString(s)
	}
	return buff.String()
}

func (f *Fixed64) Bytes() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := f.Serialize(buf)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func Fixed64FromBytes(value []byte) (*Fixed64, error) {
	var fixed64 Fixed64
	err := fixed64.Deserialize(bytes.NewReader(value))
	if err != nil {
		return nil, err
	}

	return &fixed64, nil
}

func StringToFixed64(s string) (*Fixed64, error) {
	var buffer bytes.Buffer
	//TODO: check invalid string
	di := strings.Index(s, ".")
	if len(s)-di > 9 {
		return nil, errors.New("unsupported precision")
	}
	if di == -1 {
		buffer.WriteString(s)
		for i := 0; i < 8; i++ {
			buffer.WriteByte('0')
		}
	} else {
		buffer.WriteString(s[:di])
		buffer.WriteString(s[di+1:])
		n := 8 - (len(s) - di - 1)
		for i := 0; i < n; i++ {
			buffer.WriteByte('0')
		}
	}
	r, err := strconv.ParseInt(buffer.String(), 10, 64)
	if err != nil {
		return nil, err
	}

	value := Fixed64(r)
	return &value, nil
}
