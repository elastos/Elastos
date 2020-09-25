package mempool

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.ID/types"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	memp "github.com/elastos/Elastos.ELA.SideChain/mempool"
	sctype "github.com/elastos/Elastos.ELA.SideChain/types"
)

const SlotRegisterDID = "registerdid"

func New(cfg *memp.Config) *memp.TxPool {
	txPool := memp.New(cfg)
	txPool.AddConflictSlot(&memp.Conflict{
		Name: SlotRegisterDID,
		Slot: memp.NewConflictSlot(memp.Str,
			memp.KeyTypeFuncPair{
				Type: types.RegisterDID,
				Func: addRegisterDIDTransactionHash,
			},
		),
	})
	return txPool
}

func addRegisterDIDTransactionHash(
	chain *blockchain.BlockChain, tx *sctype.Transaction) (interface{}, error) {
	regPayload, ok := tx.Payload.(*types.Operation)
	if !ok {
		return nil, errors.New("convert the payload of register did tx failed")
	}
	return regPayload.PayloadInfo.ID, nil
}
