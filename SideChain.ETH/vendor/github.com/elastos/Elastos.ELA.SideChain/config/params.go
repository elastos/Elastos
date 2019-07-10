package config

import (
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
)

type Params struct {
	// Name defines a human-readable identifier for the network.
	Name string

	// Magic defines the magic number of the peer-to-peer network.
	Magic uint32

	// DefaultPort defines the default peer-to-peer port for the network.
	DefaultPort uint16

	// DNSSeeds defines a list of DNS seeds for the network to discover peers.
	DNSSeeds []string

	// The interface/port to listen for connections
	// (default all interfaces port: 20608, testnet: 21608)
	ListenAddrs []string

	// Foundation defines the foundation address which receiving mining
	// rewards.
	Foundation common.Uint168

	// ElaAssetId
	ElaAssetId common.Uint256

	// GenesisBlock defines the first block of the chain.
	GenesisBlock *types.Block

	// PowLimit defines the highest allowed proof of work value for a block
	// as a uint256.
	PowLimit *big.Int

	// PowLimitBits defines the highest allowed proof of work value for a
	// block in compact form.
	PowLimitBits uint32

	// TargetTimespan is the desired amount of time that should elapse
	// before the block difficulty requirement is examined to determine how
	// it should be changed in order to maintain the desired block
	// generation rate.
	TargetTimespan time.Duration

	// TargetTimePerBlock is the desired amount of time to generate each
	// block.
	TargetTimePerBlock time.Duration

	// AdjustmentFactor is the adjustment factor used to limit the minimum
	// and maximum amount of adjustment that can occur between difficulty
	// retargets.
	AdjustmentFactor int64

	// CoinbaseMaturity is the number of blocks required before newly mined
	// coins (coinbase transactions) can be spent.
	CoinbaseMaturity uint32

	// Disable transaction filter supports, include bloom filter tx type filter
	// etc.
	DisableTxFilters bool

	// MinTransactionFee defines the minimum fee of a transaction.
	MinTransactionFee int64

	// ExchangeRate defines the exchange rate when transfer ELA from main chain to
	// this side chain.
	ExchangeRate float64

	// MinCrossChainTxFee defines the minimum transaction fee of a cross chain
	// transaction.
	MinCrossChainTxFee int64

	// CheckPowHeaderHeight defines the height where stating check if pow is coming
	// from main chain.
	CheckPowHeaderHeight uint32
}
