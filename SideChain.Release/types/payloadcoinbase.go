package types

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const PayloadCoinBaseVersion byte = 0x04

type PayloadCoinBase struct {
	CoinbaseData []byte
}

func (a *PayloadCoinBase) Data(version byte) []byte {
	return a.CoinbaseData
}

func (a *PayloadCoinBase) Serialize(w io.Writer, version byte) error {
	return common.WriteVarBytes(w, a.CoinbaseData)
}

func (a *PayloadCoinBase) Deserialize(r io.Reader, version byte) error {
	temp, err := common.ReadVarBytes(r, MaxPayloadDataSize,
		"payload coinbase data")
	a.CoinbaseData = temp
	return err
}
