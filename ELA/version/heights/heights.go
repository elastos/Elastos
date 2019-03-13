package heights

import (
	"errors"
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version/blocks"
	"github.com/elastos/Elastos.ELA/version/txs"
)

type TxCheckMethod func(txs.TxVersion) error
type BlockCheckMethod func(blocks.BlockVersion) (bool, bool, error)
type BlockConfirmCheckMethod func(blocks.BlockVersion) (bool, bool, error)

type VersionInfo struct {
	DefaultTxVersion        byte
	DefaultBlockVersion     uint32
	CompatibleTxVersions    map[byte]txs.TxVersion
	CompatibleBlockVersions map[uint32]blocks.BlockVersion
}

type HeightVersions struct {
	txVersionBoundary uint32
	versions          map[uint32]VersionInfo
	sortedHeights     []uint32
}

func (h *HeightVersions) GetDefaultTxVersion(blockHeight uint32) byte {
	heightKey := h.findLastAvailableHeightKey(blockHeight)
	return h.versions[heightKey].DefaultTxVersion
}

func (h *HeightVersions) GetDefaultBlockVersion(blockHeight uint32) uint32 {
	heightKey := h.findLastAvailableHeightKey(blockHeight)
	return h.versions[heightKey].DefaultBlockVersion
}

func (h *HeightVersions) CheckConfirmedBlockOnFork(block *types.Block) error {
	_, _, err := h.checkBlock(block, func(version blocks.BlockVersion) (bool, bool, error) {
		err := version.CheckConfirmedBlockOnFork(block)
		return false, false, err
	})
	return err
}

func (h *HeightVersions) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	_, _, err := h.checkBlock(block, func(version blocks.BlockVersion) (bool, bool, error) {
		err := version.AssignCoinbaseTxRewards(block, totalReward)
		return false, false, err
	})
	return err
}

func (h *HeightVersions) checkTxCompatibleWithLowVersion(blockHeight uint32, tx *types.Transaction, txFun TxCheckMethod) error {
	if tx == nil {
		return errors.New("Transaction is null")
	}

	heightKey := h.findLastAvailableHeightKey(blockHeight)
	info := h.versions[heightKey]

	txVersion := h.findTxVersion(blockHeight, &info, tx)
	if txVersion == nil && blockHeight < h.txVersionBoundary {
		txVersion = info.CompatibleTxVersions[info.DefaultTxVersion]
	}

	if txVersion == nil {
		return fmt.Errorf("Block height %d can not support transaction version %d", blockHeight, tx.Version)
	}
	return txFun(txVersion)
}

func (h *HeightVersions) findTxVersion(blockHeight uint32, info *VersionInfo, tx *types.Transaction) txs.TxVersion {
	return info.CompatibleTxVersions[byte(tx.Version)]
}

func (h *HeightVersions) checkBlock(block *types.Block, blockFun BlockCheckMethod) (bool, bool, error) {
	heightKey := h.findLastAvailableHeightKey(block.Height)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, block.Version)
	if v == nil {
		return false, false, fmt.Errorf("[checkBlock] Block height %d can not support block version %d", block.Height, block.Version)
	}
	return blockFun(v)
}

func (h *HeightVersions) checkDposBlock(dposBlock *types.DposBlock, blockConfirmFun BlockConfirmCheckMethod) (bool, bool, error) {
	heightKey := h.findLastAvailableHeightKey(dposBlock.Block.Height)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, dposBlock.Block.Version)
	if v == nil {
		return false, false, fmt.Errorf("[checkBlockConfirm] Block height %d can not support block version %d",
			dposBlock.Block.Height, dposBlock.Block.Version)
	}
	return blockConfirmFun(v)
}

func (h *HeightVersions) findBlockVersion(info *VersionInfo, version uint32) blocks.BlockVersion {
	return info.CompatibleBlockVersions[version]
}

func (h *HeightVersions) findLastAvailableHeightKey(blockHeight uint32) uint32 {
	for i := 0; i < len(h.sortedHeights)-1; i++ {
		if blockHeight >= h.sortedHeights[i] && blockHeight < h.sortedHeights[i+1] {
			return h.sortedHeights[i]
		}
	}

	return h.sortedHeights[len(h.sortedHeights)-1]
}

func NewHeightVersions(versions map[uint32]VersionInfo, txVersionBoundary uint32) *HeightVersions {
	h := &HeightVersions{
		versions:          versions,
		sortedHeights:     []uint32{},
		txVersionBoundary: txVersionBoundary,
	}

	var sortedHeights sort.IntSlice
	for k := range h.versions {
		sortedHeights = append(sortedHeights, int(k))
	}
	sortedHeights.Sort()
	for _, height := range sortedHeights {
		h.sortedHeights = append(h.sortedHeights, uint32(height))
	}
	return h
}
