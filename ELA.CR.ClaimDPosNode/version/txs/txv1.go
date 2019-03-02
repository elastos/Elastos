package txs

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
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

func (v *txV1) CheckOutputProgramHash(programHash common.Uint168) error {
	var empty = common.Uint168{}
	if programHash.IsEqual(empty) {
		return nil
	}

	prefix := contract.PrefixType(programHash[0])
	switch prefix {
	case contract.PrefixStandard:
	case contract.PrefixMultiSig:
	case contract.PrefixCrossChain:
	case contract.PrefixDeposit:
	default:
		return errors.New("invalid program hash prefix")
	}

	addr, err := programHash.ToAddress()
	if err != nil {
		return errors.New("invalid program hash")
	}
	_, err = common.Uint168FromAddress(addr)
	if err != nil {
		return errors.New("invalid program hash")
	}

	return nil
}

func NewTxV1(cfg *verconf.Config) *txV1 {
	return &txV1{NewTxV0(cfg)}
}
