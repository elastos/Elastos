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
	txMax := txs.NewTxV2(cfg)

	blockV0 := blocks.NewBlockV0(cfg)
	blockV1 := blocks.NewBlockV1(cfg)
	blockMax := blocks.NewBlockV2(cfg)

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
				map[byte]txs.TxVersion{txMax.GetVersion(): txMax},
				map[uint32]blocks.BlockVersion{blockV1.GetVersion(): blockV1},
			},
			heights.HeightVersion3: {
				9,
				2,
				map[byte]txs.TxVersion{txMax.GetVersion(): txMax},
				map[uint32]blocks.BlockVersion{blockMax.GetVersion(): blockMax},
			},
		},
	)
	cfg.Versions = versions
	return versions
}
