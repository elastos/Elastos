package version

import (
	"os"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/version/blocks"
	"github.com/elastos/Elastos.ELA/version/heights"
	"github.com/elastos/Elastos.ELA/version/txs"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

const (
	versionCount = 4
)

func NewVersions(cfg *verconf.Config) interfaces.HeightVersions {
	if len(cfg.ChainParams.HeightVersions) < versionCount {
		log.Fatal("insufficient height version count")
		os.Exit(1)
	}

	txV0 := txs.NewTxV0(cfg)
	txV1 := txs.NewTxV1(cfg)
	txV2 := txs.NewTxV2(cfg)
	txMax := txs.NewTxV3(cfg)

	blockV0 := blocks.NewBlockV0(cfg)
	blockV1 := blocks.NewBlockV1(cfg)
	blockMax := blocks.NewBlockV2(cfg)

	versions := heights.NewHeightVersions(
		map[uint32]heights.VersionInfo{
			cfg.ChainParams.HeightVersions[0]: {
				0,
				0,
				map[byte]txs.TxVersion{txV0.GetVersion(): txV0},
				map[uint32]blocks.BlockVersion{blockV0.GetVersion(): blockV0},
			},
			cfg.ChainParams.HeightVersions[1]: {
				1,
				0,
				map[byte]txs.TxVersion{txV1.GetVersion(): txV1},
				map[uint32]blocks.BlockVersion{blockV0.GetVersion(): blockV0},
			},
			cfg.ChainParams.HeightVersions[2]: {
				9,
				1,
				map[byte]txs.TxVersion{txV2.GetVersion(): txV2},
				map[uint32]blocks.BlockVersion{blockV1.GetVersion(): blockV1},
			},
			cfg.ChainParams.HeightVersions[3]: {
				10,
				2,
				map[byte]txs.TxVersion{txV2.GetVersion(): txV2, txMax.GetVersion(): txMax},
				map[uint32]blocks.BlockVersion{blockMax.GetVersion(): blockMax},
			},
		},
		cfg.ChainParams.HeightVersions[2],
	)
	return versions
}
