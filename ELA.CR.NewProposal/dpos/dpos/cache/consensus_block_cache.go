package cache

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	. "github.com/elastos/Elastos.ELA/dpos/chain"
)

type ConsensusBlockCacheListener interface {
	OnBlockAdded(b *Block)
}

type ConsensusBlockCache struct {
	ConsensusBlocks    map[common.Uint256]*Block
	ConsensusBlockList []common.Uint256

	Listener ConsensusBlockCacheListener
}

func (c *ConsensusBlockCache) Reset() {
	c.ConsensusBlocks = make(map[common.Uint256]*Block)
	c.ConsensusBlockList = make([]common.Uint256, 0)
}

func (c *ConsensusBlockCache) AddValue(key common.Uint256, value *Block) {
	c.ConsensusBlocks[key] = value
	c.ConsensusBlockList = append(c.ConsensusBlockList, key)

	if c.Listener != nil {
		c.Listener.OnBlockAdded(value)
	}
}

func (c *ConsensusBlockCache) TryGetValue(key common.Uint256) (*Block, bool) {
	value, ok := c.ConsensusBlocks[key]

	return value, ok
}

func (c *ConsensusBlockCache) GetFirstArrivedBlockHash() (common.Uint256, bool) {
	if len(c.ConsensusBlockList) == 0 {
		return common.Uint256{}, false
	}
	return c.ConsensusBlockList[0], true
}
