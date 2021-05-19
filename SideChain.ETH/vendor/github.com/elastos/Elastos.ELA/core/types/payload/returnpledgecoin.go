package payload

import "io"

const ReturnDepositCoinVersion byte = 0x00

type ReturnDepositCoin struct {
}

func (a *ReturnDepositCoin) Data(version byte) []byte {
	return nil
}

func (a *ReturnDepositCoin) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *ReturnDepositCoin) Deserialize(r io.Reader, version byte) error {
	return nil
}
