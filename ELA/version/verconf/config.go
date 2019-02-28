package verconf

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/mempool"
)

type Config struct {
	Server       elanet.Server
	Chain        *blockchain.BlockChain
	ChainStore   blockchain.IChainStore
	ChainParams  *config.Params
	TxMemPool    *mempool.TxPool
	BlockMemPool *mempool.BlockPool
	Arbitrators  interfaces.Arbitrators
	Versions     interfaces.HeightVersions
}
