package _interface

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
)

type QueueItem struct {
	TxHash        common.Uint256
	BlockHash     common.Uint256
	Height        uint32
}
