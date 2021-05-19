package manager

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type ConsensusBlockCacheListener interface {
	OnBlockAdded(b *types.Block)
}

type ConsensusBlockCache struct {
	ConsensusBlocks    map[common.Uint256]*types.Block
	ConsensusBlockList []common.Uint256

	Listener ConsensusBlockCacheListener
}

func (c *ConsensusBlockCache) Reset(block *types.Block) {
	if block == nil {
		c.ConsensusBlocks = make(map[common.Uint256]*types.Block)
		c.ConsensusBlockList = make([]common.Uint256, 0)
		return
	}

	newConsensusBlocks := make(map[common.Uint256]*types.Block)
	newConsensusBlockList := make([]common.Uint256, 0)
	for _, b := range c.ConsensusBlocks {
		if b.Height < block.Height {
			continue
		}
		newConsensusBlocks[b.Hash()] = b
		newConsensusBlockList = append(c.ConsensusBlockList, b.Hash())
	}
	c.ConsensusBlocks = newConsensusBlocks
	c.ConsensusBlockList = newConsensusBlockList
}

func (c *ConsensusBlockCache) AddValue(key common.Uint256, value *types.Block) {
	c.ConsensusBlocks[key] = value
	c.ConsensusBlockList = append(c.ConsensusBlockList, key)

	if c.Listener != nil {
		c.Listener.OnBlockAdded(value)
	}
}

func (c *ConsensusBlockCache) TryGetValue(key common.Uint256) (*types.Block, bool) {
	value, ok := c.ConsensusBlocks[key]

	return value, ok
}

func (c *ConsensusBlockCache) GetFirstArrivedBlockHash() (common.Uint256, bool) {
	if len(c.ConsensusBlockList) == 0 {
		return common.Uint256{}, false
	}
	return c.ConsensusBlockList[0], true
}
