package version

import (
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/version/blocks"
	"github.com/elastos/Elastos.ELA/version/heights"
	"github.com/elastos/Elastos.ELA/version/txs"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

func NewVersions(cfg *verconf.Config) interfaces.HeightVersions {
	txV0 := txs.NewTxV0(cfg)
	txV1 := txs.NewTxV1(cfg)
	txCurrent := txs.NewTxCurrent(cfg)

	blockV0 := blocks.NewBlockV0(cfg)
	blockCurrent := blocks.NewBlockCurrent(cfg)

	versions := heights.NewHeightVersions(
		map[uint32]heights.VersionInfo{
			heights.GenesisHeightVersion: {
				0,
				0,
				map[byte]txs.TxVersion{txV0.GetVersion(): txV0},
				map[uint32]blocks.BlockVersion{blockV0.GetVersion(): blockV0},
			},
			heights.HeightVersion1: {
				1,
				0,
				map[byte]txs.TxVersion{txV1.GetVersion(): txV1},
				map[uint32]blocks.BlockVersion{blockV0.GetVersion(): blockV0},
			},
			heights.HeightVersion2: {
				9,
				1,
				map[byte]txs.TxVersion{txCurrent.GetVersion(): txCurrent},
				map[uint32]blocks.BlockVersion{blockCurrent.GetVersion(): blockCurrent},
			},
		},
	)
	cfg.Versions = versions
	return versions
}
