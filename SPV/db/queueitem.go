package db

import (
	. "SPVWallet/core"
)

type QueueItem struct {
	TxHash        Uint256
	BlockHash     Uint256
	Height        uint32
	ConfirmHeight uint32
}
