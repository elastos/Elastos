package verconfig

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	. "github.com/elastos/Elastos.ELA/version"
	"github.com/elastos/Elastos.ELA/version/blockhistory"
	. "github.com/elastos/Elastos.ELA/version/heights"
	"github.com/elastos/Elastos.ELA/version/txhistory"
)

func InitVersions() blockchain.HeightVersions {
	txV0 := &txhistory.TxVersionV0{}
	txV1 := &txhistory.TxVersionV1{}
	txVCurrent := &TxVersionMain{}

	blockV0 := &blockhistory.BlockVersionV0{}
	blockVCurrent := &BlockVersionMain{}

	return NewHeightVersions(
		map[uint32]VersionInfo{
			GenesisHeightVersion: {
				0,
				0,
				map[byte]TxVersion{txV0.GetVersion(): txV0},
				map[uint32]BlockVersion{blockV0.GetVersion(): blockV0},
			},
			HeightVersion1: {
				1,
				0,
				map[byte]TxVersion{txV1.GetVersion(): txV1},
				map[uint32]BlockVersion{blockV0.GetVersion(): blockV0},
			},
			HeightVersion2: {
				9,
				1,
				map[byte]TxVersion{txVCurrent.GetVersion(): txVCurrent},
				map[uint32]BlockVersion{blockVCurrent.GetVersion(): blockVCurrent},
			},
		},
	)
}
