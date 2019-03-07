package txs

import (
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV1 implement the TxVersion interface.
var _ TxVersion = (*txV0)(nil)

type txV0 struct {
	cfg *verconf.Config
}

func (v *txV0) GetVersion() byte {
	return 0
}

func NewTxV0(cfg *verconf.Config) *txV0 {
	return &txV0{cfg: cfg}
}
