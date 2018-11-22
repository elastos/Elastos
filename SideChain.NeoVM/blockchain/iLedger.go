package blockchain

import (
	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
)

type ILedgerStore interface {
	database.Database
	GetUnspents(txid common.Uint256) ([]*types.Output, error)
	GetTxReference(tx *types.Transaction) (map[*types.Input]*types.Output, error)
	GetAccount(programHash *common.Uint168) (*states.AccountState, error)
}
