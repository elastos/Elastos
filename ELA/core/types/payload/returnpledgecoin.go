package payload

import "io"

const PayloadReturnDepositCoinVersion byte = 0x00

type PayloadReturnDepositCoin struct {
}

func (a *PayloadReturnDepositCoin) Data(version byte) []byte {
	return nil
}

func (a *PayloadReturnDepositCoin) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *PayloadReturnDepositCoin) Deserialize(r io.Reader, version byte) error {
	return nil
}
