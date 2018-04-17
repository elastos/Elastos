package _interface

import (
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type QueueItem struct {
	TxHash        Uint256
	BlockHash     Uint256
	Height        uint32
}
