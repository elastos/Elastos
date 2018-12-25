package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/interface/iutil"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
)

func newBlockHeader() util.BlockHeader {
	return iutil.NewHeader(&types.Header{})
}

func newTransaction() util.Transaction {
	return iutil.NewTx(&types.Transaction{})
}

// GenesisHeader creates a specific genesis header by the given
// foundation address.
func GenesisHeader(foundation *common.Uint168) util.BlockHeader {
	return iutil.NewHeader(&config.GenesisBlock(foundation).Header)
}
