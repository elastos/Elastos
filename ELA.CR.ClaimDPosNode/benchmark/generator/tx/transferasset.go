package tx

import (
	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/benchmark/generator/utils"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const (
	nonceByteLength = 20
)

type transferAssetGenerator struct {
	account []*account.Account
}

func (g *transferAssetGenerator) Generate() *types.Transaction {
	txn := &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.TransferAsset,
		PayloadVersion: 0,
		Payload:        &payload.TransferAsset{},
		Attributes: []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  utils.RandomBytes(nonceByteLength),
			},
		},
		Inputs:   nil,
		Outputs:  []*types.Output{},
		LockTime: 0,
		Programs: nil,
	}
	for _, v := range g.account {
		txn.Outputs = append(txn.Outputs, &types.Output{
			AssetID:     config.ELAAssetID,
			Value:       0, // assign later
			OutputLock:  0,
			ProgramHash: v.ProgramHash,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		}, )
	}

	return txn
}
