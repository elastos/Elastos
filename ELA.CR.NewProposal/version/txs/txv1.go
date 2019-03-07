package txs

import (
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV1 implement the TxVersion interface.
var _ TxVersion = (*txV1)(nil)

type txV1 struct {
	*txV0
}

func (v *txV1) GetVersion() byte {
	return 1
}

func NewTxV1(cfg *verconf.Config) *txV1 {
	return &txV1{NewTxV0(cfg)}
}
