package sync

import (
	"github.com/elastos/Elastos.ELA.SPV/blockchain"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	defaultMaxPeers = 125
)

// Config is a configuration struct used to initialize a new SyncManager.
type Config struct {
	Chain          *blockchain.BlockChain
	MaxPeers       int
	CandidateFlags []uint64

	GetTxFilter         func() *msg.TxFilterLoad
	TransactionAnnounce func(tx util.Transaction)
}

func NewDefaultConfig(chain *blockchain.BlockChain, candidateFlags []uint64,
	getTxFilter func() *msg.TxFilterLoad) *Config {
	return &Config{
		Chain:          chain,
		CandidateFlags: candidateFlags,
		MaxPeers:       defaultMaxPeers,
		GetTxFilter:    getTxFilter,
	}
}
