package txs

import (
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV2 implement the TxVersion interface.
var _ TxVersion = (*txV2)(nil)

// txV2 represent the current transaction version.
type txV2 struct {
	*txV1
}

func (v *txV2) GetVersion() byte {
	return 9
}

func NewTxV2(cfg *verconf.Config) *txV2 {
	return &txV2{NewTxV1(cfg)}
}
