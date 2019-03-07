package txs

import (
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV2 implement the TxVersion interface.
var _ TxVersion = (*txV3)(nil)

// txV2 represent the current transaction version.
type txV3 struct {
	*txV2
}

func (v *txV3) GetVersion() byte {
	return 10
}

func NewTxV3(cfg *verconf.Config) *txV3 {
	return &txV3{NewTxV2(cfg)}
}
