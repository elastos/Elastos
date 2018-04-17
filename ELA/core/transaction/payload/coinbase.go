package payload

import (
	"io"

	. "github.com/elastos/Elastos.ELA/common/serialize"
)

const CoinBasePayloadVersion byte = 0x04

type CoinBase struct {
	CoinbaseData []byte
}

func (a *CoinBase) Data(version byte) []byte {
	return a.CoinbaseData
}

func (a *CoinBase) Serialize(w io.Writer, version byte) error {
	return WriteVarBytes(w, a.CoinbaseData)
}

func (a *CoinBase) Deserialize(r io.Reader, version byte) error {
	temp, err := ReadVarBytes(r)
	a.CoinbaseData = temp
	return err
}
