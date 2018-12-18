package config

import (
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

// These variables are the chain proof-of-work limit parameters for each default
// network.
var (
	// bigOne is 1 represented as a big.Int.  It is defined here to avoid
	// the overhead of creating it multiple times.
	bigOne = big.NewInt(1)

	// powLimit is the highest proof of work value a block can have for the network.
	//  It is the value 2^255 - 1.
	powLimit = new(big.Int).Sub(new(big.Int).Lsh(bigOne, 255), bigOne)

	// "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
	mainNetFoundation = common.Uint168{
		0x12, 0x9e, 0x9c, 0xf1, 0xc5, 0xf3, 0x36,
		0xfc, 0xf3, 0xa6, 0xc9, 0x54, 0x44, 0x4e,
		0xd4, 0x82, 0xc5, 0xd9, 0x16, 0xe5, 0x06,
	}

	// "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3"
	testNetFoundation = common.Uint168{
		0x12, 0xc8, 0xa2, 0xe0, 0x67, 0x72, 0x27,
		0x14, 0x4d, 0xf8, 0x22, 0xb7, 0xd9, 0x24,
		0x6c, 0x58, 0xdf, 0x68, 0xeb, 0x11, 0xce,
	}
)

// MainNetParams defines the network parameters for the main network.
var MainNetParams = Params{
	Name:        "mainnet",
	Magic:       2017001,
	DefaultPort: 20338,

	SeedList: []string{
		"node-mainnet-002.elastos.org",
		"node-mainnet-003.elastos.org",
		"node-mainnet-004.elastos.org",
		"node-mainnet-006.elastos.org",
		"node-mainnet-007.elastos.org",
		"node-mainnet-014.elastos.org",
		"node-mainnet-015.elastos.org",
		"node-mainnet-016.elastos.org",
		"node-mainnet-017.elastos.org",
		"node-mainnet-022.elastos.org",
		"node-mainnet-021.elastos.org",
		"node-mainnet-023.elastos.org",
	},

	Foundation:         mainNetFoundation,
	GenesisBlock:       GenesisBlock(mainNetFoundation),
	PowLimit:           powLimit,
	PowLimitBits:       0x1f0008ff,
	TargetTimespan:     24 * time.Hour,  // 24 hours
	TargetTimePerBlock: 2 * time.Minute, // 2 minute
	AdjustmentFactor:   4,               // 25% less, 400% more
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
}

// TestNetParams defines the network parameters for the test network.
var TestNetParams = Params{
	Name:        "testnet",
	Magic:       2018001,
	DefaultPort: 21338,

	SeedList: []string{
		"node-testnet-001.elastos.org",
		"node-testnet-002.elastos.org",
		"node-testnet-003.elastos.org",
		"node-testnet-004.elastos.org",
		"node-testnet-005.elastos.org",
	},

	Foundation:         testNetFoundation,
	GenesisBlock:       GenesisBlock(testNetFoundation),
	PowLimit:           powLimit,
	PowLimitBits:       0x1f0008ff,
	TargetTimespan:     24 * time.Hour,  // 24 hours
	TargetTimePerBlock: 2 * time.Minute, // 2 minute
	AdjustmentFactor:   4,               // 25% less, 400% more
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
}

// RegNetParams defines the network parameters for the regression test network.
var RegNetParams = Params{
	Name:               "regnet",
	Foundation:         testNetFoundation,
	GenesisBlock:       GenesisBlock(testNetFoundation),
	PowLimit:           powLimit,
	PowLimitBits:       0x207fffff,
	TargetTimePerBlock: time.Second * 1,      // 1 second
	TargetTimespan:     time.Second * 1 * 10, // 10 seconds
	AdjustmentFactor:   4,                    // 25% less, 400% more
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
}

type Params struct {
	// Name defines a human-readable identifier for the network.
	Name string

	// Magic defines the magic number of the peer-to-peer network.
	Magic uint32

	// DefaultPort defines the default peer-to-peer port for the network.
	DefaultPort uint16

	// SeedList defines a list of seed peers.
	SeedList []string

	// The interface/port to listen for connections.
	ListenAddrs []string

	// Foundation defines the foundation address which receiving mining
	// rewards.
	Foundation common.Uint168

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
}
