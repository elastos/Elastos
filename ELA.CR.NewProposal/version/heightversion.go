package version

import (
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	GenesisHeightVersion = uint32(0)
	HeightVersion1       = uint32(88812)
	HeightVersion2       = uint32(108812) //fixme edit height later
)

type TxCheckMethod func(TxVersion) error
type BlockCheckMethod func(BlockVersion) error

type VersionInfo struct {
	CompatibleTxVersions    map[byte]TxVersion
	CompatibleBlockVersions map[uint32]BlockVersion
}

type heightVersions struct {
	versions      map[uint32]VersionInfo
	sortedHeights []uint32
}

func (h *heightVersions) CheckOutputPayload(blockHeight uint32, tx *core.Transaction, output *core.Output) error {
	return h.checkTx(blockHeight, tx, func(v TxVersion) error {
		return v.CheckOutputPayload(output)
	})
}

func (h *heightVersions) CheckOutputProgramHash(blockHeight uint32, tx *core.Transaction, programHash Uint168) error {
	return h.checkTx(blockHeight, tx, func(v TxVersion) error {
		return v.CheckOutputProgramHash(programHash)
	})
}

func (h *heightVersions) CheckCoinbaseMinerReward(blockHeight uint32, tx *core.Transaction, totalReward Fixed64) error {
	return h.checkTx(blockHeight, tx, func(version TxVersion) error {
		return version.CheckCoinbaseMinerReward(tx, totalReward)
	})
}

func (h *heightVersions) CheckCoinbaseArbitratorsReward(blockHeight uint32, tx *core.Transaction, rewardInCoinbase Fixed64) error {
	return h.checkTx(blockHeight, tx, func(version TxVersion) error {
		return version.CheckCoinbaseArbitratorsReward(tx, rewardInCoinbase)
	})
}

func (h *heightVersions) CheckVoteProducerOutputs(blockHeight uint32, tx *core.Transaction, outputs []*core.Output, references map[*core.Input]*core.Output) error {
	return h.checkTx(blockHeight, tx, func(version TxVersion) error {
		return version.CheckVoteProducerOutputs(outputs, references)
	})
}

func (h *heightVersions) GetProducersDesc(block *core.Block) ([][]byte, error) {
	heightKey := h.findLastAvailableHeightKey(block.Height)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, block)
	if v == nil {
		return nil, fmt.Errorf("Block height ", block.Height, "can not support block version ", block.Version)
	}
	return v.GetProducersDesc()
}

func (h *heightVersions) checkTx(blockHeight uint32, tx *core.Transaction, txFun TxCheckMethod) error {
	heightKey := h.findLastAvailableHeightKey(blockHeight)
	info := h.versions[heightKey]

	v := h.findTxVersion(blockHeight, &info, tx)
	if v == nil {
		return fmt.Errorf("Block height ", blockHeight, "can not support transaction version ", tx.Version)
	}
	return txFun(v)
}

func (h *heightVersions) findTxVersion(blockHeight uint32, info *VersionInfo, tx *core.Transaction) TxVersion {
	// before HeightVersion2 tx version means tx type, use special get method instead
	if blockHeight < HeightVersion2 {
		if blockHeight < HeightVersion1 {
			return info.CompatibleTxVersions[0]
		} else {
			return info.CompatibleTxVersions[1]
		}
	}

	v, ok := info.CompatibleTxVersions[byte(tx.Version)]
	if !ok {
		return nil
	} else {
		return v
	}
}

func (h *heightVersions) checkBlock(blockHeight uint32, block *core.Block, blockFun BlockCheckMethod) error {
	heightKey := h.findLastAvailableHeightKey(blockHeight)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, block)
	if v == nil {
		return fmt.Errorf("Block height ", blockHeight, "can not support block version ", block.Version)
	}
	return blockFun(v)
}

func (h *heightVersions) findBlockVersion(info *VersionInfo, block *core.Block) BlockVersion {
	v, ok := info.CompatibleBlockVersions[block.Version]
	if !ok {
		return nil
	} else {
		return v
	}
}

func (h *heightVersions) findLastAvailableHeightKey(blockHeight uint32) uint32 {
	for i := 0; i < len(h.sortedHeights)-1; i++ {
		if blockHeight >= h.sortedHeights[i] && blockHeight < h.sortedHeights[i+1] {
			return h.sortedHeights[i]
		}
	}

	return h.sortedHeights[len(h.sortedHeights)-1]
}

func NewHeightVersions(versions map[uint32]VersionInfo) blockchain.HeightVersions {

	h := &heightVersions{
		versions:      versions,
		sortedHeights: []uint32{},
	}

	for k := range h.versions {
		h.sortedHeights = append(h.sortedHeights, k)
	}
	sort.Slice(h.sortedHeights, func(i, j int) bool {
		return i < j
	})

	return h
}
