package db

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
)

type QueueItem struct {
	NotifyId common.Uint256
	TxId     common.Uint256
	Height   uint32
}
