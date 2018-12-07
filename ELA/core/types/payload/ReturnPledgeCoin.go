package payload

import "io"

const PayloadReturnPledgeCoinVersion byte = 0x00

type PayloadReturnPledgeCoin struct {
}

func (a *PayloadReturnPledgeCoin) Data(version byte) []byte {
	return nil
}

func (a *PayloadReturnPledgeCoin) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *PayloadReturnPledgeCoin) Deserialize(r io.Reader, version byte) error {
	return nil
}
