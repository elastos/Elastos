package core

import "sort"

const (
	CheckTxOut                = uint64(1) << 1
	CheckCoinbaseTxDposReward = uint64(1) << 2
)

var (
	BlockHeightVersions HeightVersions
)

type VersionInfo struct {
	TxCheckFlag uint64
	Description string
}

type HeightVersions interface {
	GetCheckFlags(blockHeight uint32) uint64
}

type heightVersions struct {
	versions      map[uint32]VersionInfo
	sortedHeights []uint32
}

func (h *heightVersions) GetCheckFlags(blockHeight uint32) uint64 {
	key := h.findLastAvailableVersionKey(blockHeight)
	versionInfo, ok := h.versions[key]
	if !ok {
		return 0
	}

	return versionInfo.TxCheckFlag
}

func (h *heightVersions) findLastAvailableVersionKey(blockHeight uint32) uint32 {
	for i := 0; i < len(h.sortedHeights)-1; i++ {
		if blockHeight >= h.sortedHeights[i] && blockHeight < h.sortedHeights[i+1] {
			return h.sortedHeights[i]
		}
	}

	return h.sortedHeights[len(h.sortedHeights)-1]
}

func init() {
	h := &heightVersions{
		versions: map[uint32]VersionInfo{
			88812:  {CheckTxOut, "check transaction output"},
			108812: {CheckTxOut | CheckCoinbaseTxDposReward, "check dpos reward in coinbase transaction"}, //fixme height edit  later
		},
		sortedHeights: []uint32{0},
	}

	for k := range h.versions {
		h.sortedHeights = append(h.sortedHeights, k)
	}
	sort.Slice(h.sortedHeights, func(i, j int) bool {
		return i < j
	})

	BlockHeightVersions = h
}
