package db

import (
	"bytes"
	"math/big"

	"github.com/elastos/Elastos.ELA.SPV/core"
)

type Header struct {
	core.Header
	TotalWork *big.Int
}

func (sh *Header) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := sh.Header.Serialize(buf)
	if err != nil {
		return nil, err
	}

	biBytes := sh.TotalWork.Bytes()
	pad := make([]byte, 32-len(biBytes))
	serializedBI := append(pad, biBytes...)
	buf.Write(serializedBI)
	return buf.Bytes(), nil
}

func (sh *Header) Deserialize(b []byte) error {
	r := bytes.NewReader(b)
	err := sh.Header.Deserialize(r)
	if err != nil {
		return err
	}

	biBytes := make([]byte, 32)
	_, err = r.Read(biBytes)
	if err != nil {
		return err
	}
	sh.TotalWork = new(big.Int)
	sh.TotalWork.SetBytes(biBytes)

	return nil
}
