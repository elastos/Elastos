package db

import (
	. "github.com/elastos/Elastos.ELA.SPV/common"
)

type QueueItem struct {
	TxHash        Uint256
	BlockHash     Uint256
	Height        uint32
	ConfirmHeight uint32
}
