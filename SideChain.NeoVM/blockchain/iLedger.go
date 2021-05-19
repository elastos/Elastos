package blockchain

import (
	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	ntype "github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"

)

type ILedgerStore interface {
	database.Database
	GetUnspents(txid common.Uint256) ([]*types.Output, error)
	GetTxReference(tx *types.Transaction) (map[*types.Input]*types.Output, error)
	GetAccount(programHash *common.Uint168) (*states.AccountState, error)
    ReadTransaction(hash common.Uint256) (*types.Transaction, common.Uint256, uint32, uint32)
    GetReceipts(height uint32, hash *common.Uint256) (ntype.Receipts, error)
	GetBlock(hash common.Uint256) (*types.Block, error)
	GetHeader(hash common.Uint256) (interfaces.Header, error)
}
