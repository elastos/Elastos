package wallet

import (
	"github.com/elastos/Elastos.ELA/blockchain"
)

const (
	key             = "utxo"
	dataExtension   = "upt"
	savePeriod      = uint32(720)
	effectivePeriod = uint32(720)

	WalletVersion = "0.0.1"
)

var (
	Wal    *Wallet
	Store  blockchain.IChainStore
	CoinCP *CoinsCheckPoint
)
