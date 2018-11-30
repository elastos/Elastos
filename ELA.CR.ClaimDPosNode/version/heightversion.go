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
type BlockConfirmCheckMethod func(BlockVersion) (bool, error)

type VersionInfo struct {
	DefaultTxVersion        byte
	DefaultBlockVersion     uint32
	CompatibleTxVersions    map[byte]TxVersion
	CompatibleBlockVersions map[uint32]BlockVersion
}

type heightVersions struct {
	versions      map[uint32]VersionInfo
	sortedHeights []uint32
}

func (h *heightVersions) GetDefaultTxVersion(blockHeight uint32) byte {
	heightKey := h.findLastAvailableHeightKey(blockHeight)
	return h.versions[heightKey].DefaultTxVersion
}

func (h *heightVersions) GetDefaultBlockVersion(blockHeight uint32) uint32 {
	heightKey := h.findLastAvailableHeightKey(blockHeight)
	return h.versions[heightKey].DefaultBlockVersion
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

func (h *heightVersions) CheckTxHasNoProgramsAndAttributes(blockHeight uint32, tx *core.Transaction) error {
	return h.checkTx(blockHeight, tx, func(version TxVersion) error {
		return version.CheckTxHasNoProgramsAndAttributes(tx)
	})
}

func (h *heightVersions) GetProducersDesc(block *core.Block) ([][]byte, error) {
	heightKey := h.findLastAvailableHeightKey(block.Height)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, block)
	if v == nil {
		return nil, fmt.Errorf("[GetProducersDesc] Block height %d can not support block version %d", block.Height, block.Version)
	}
	return v.GetProducersDesc()
}

func (h *heightVersions) CheckConfirmedBlockOnFork(block *core.Block) error {
	return h.checkBlock(block, func(version BlockVersion) error {
		return version.CheckConfirmedBlockOnFork(block)
	})
}

func (h *heightVersions) AddBlock(block *core.Block) error {
	return h.checkBlock(block, func(version BlockVersion) error {
		return version.AddBlock(block)
	})
}

func (h *heightVersions) AddBlockConfirm(blockConfirm *core.BlockConfirm) (bool, error) {
	return h.checkBlockConfirm(blockConfirm, func(version BlockVersion) (bool, error) {
		return version.AddBlockConfirm(blockConfirm)
	})
}

func (h *heightVersions) AssignCoinbaseTxRewards(block *core.Block, totalReward Fixed64) error {
	return h.checkBlock(block, func(version BlockVersion) error {
		return version.AssignCoinbaseTxRewards(block, totalReward)
	})
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
		return info.CompatibleTxVersions[info.DefaultTxVersion]
	}

	v, ok := info.CompatibleTxVersions[byte(tx.Version)]
	if !ok {
		return nil
	} else {
		return v
	}
}

func (h *heightVersions) checkBlock(block *core.Block, blockFun BlockCheckMethod) error {
	heightKey := h.findLastAvailableHeightKey(block.Height)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, block)
	if v == nil {
		return fmt.Errorf("[checkBlock] Block height %d can not support block version %d", block.Height, block.Version)
	}
	return blockFun(v)
}

func (h *heightVersions) checkBlockConfirm(blockConfirm *core.BlockConfirm, blockConfirmFun BlockConfirmCheckMethod) (bool, error) {
	if blockConfirm == nil || !blockConfirm.BlockFlag {
		return false, fmt.Errorf("[checkBlockConfirm] received block confirm with nil block")
	}
	heightKey := h.findLastAvailableHeightKey(blockConfirm.Block.Height)
	info := h.versions[heightKey]

	v := h.findBlockVersion(&info, blockConfirm.Block)
	if v == nil {
		return false, fmt.Errorf("[checkBlockConfirm] Block height %d can not support block version %d", blockConfirm.Block.Height, blockConfirm.Block.Version)
	}
	return blockConfirmFun(v)
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
