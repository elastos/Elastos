package txs

import (
	"errors"
	"github.com/elastos/Elastos.ELA/core/types"
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

func (v *txV2) CheckOutputPayload(txType types.TxType, output *types.Output) error {
	// OTVote information can only be placed in TransferAsset transaction.
	if txType == types.TransferAsset {
		switch output.Type {
		case types.OTNone:
		case types.OTVote:
		default:
			return errors.New("transaction type dose not match the output payload type")
		}
	} else {
		switch output.Type {
		case types.OTNone:
		default:
			return errors.New("transaction type dose not match the output payload type")
		}
	}

	return output.Payload.Validate()
}

func (v *txV2) CheckTxHasNoPrograms(tx *types.Transaction) error {
	if len(tx.Programs) != 0 {
		return errors.New("transaction should have no programs")
	}

	return nil
}

func NewTxV2(cfg *verconf.Config) *txV2 {
	return &txV2{NewTxV1(cfg)}
}
