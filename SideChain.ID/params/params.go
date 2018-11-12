package params

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/config"
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

	// "8NRxtbMKScEWzW8gmPDGUZ8LSzm688nkZZ"
	testNetFoundation = common.Uint168{
		0x12, 0x50, 0x96, 0x58, 0xd3, 0x9e, 0x4b,
		0xde, 0x30, 0x79, 0xe3, 0xf8, 0xde, 0x91,
		0xf4, 0x9c, 0xaa, 0x97, 0x01, 0x5c, 0x9e,
	}
)

// MainNetSpvParams defines the network parameters for the main network SPV.
var MainNetSpvParams = config.SpvParams{
	Magic:      2017001,
	Foundation: "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",

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

	DefaultPort: 20866,
}

// TestNetSpvParams defines the network parameters for the test network SPV.
var TestNetSpvParams = config.SpvParams{
	Magic:      2018001,
	Foundation: "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",

	SeedList: []string{
		"node-testnet-001.elastos.org",
		"node-testnet-002.elastos.org",
		"node-testnet-003.elastos.org",
		"node-testnet-004.elastos.org",
		"node-testnet-005.elastos.org",
	},

	DefaultPort: 21866,
}

// RegNetSpvParams defines the network parameters for the regression network SPV.
var RegNetSpvParams = config.SpvParams{
	Foundation: "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",
}

// MainNetParams defines the network parameters for the main network.
var MainNetParams = config.Params{
	Name:        "mainnet",
	Magic:       2018001,
	DefaultPort: 20608,

	SeedList: []string{
		"did-mainnet-001.elastos.org",
		"did-mainnet-002.elastos.org",
		"did-mainnet-003.elastos.org",
		"did-mainnet-004.elastos.org",
		"did-mainnet-005.elastos.org",
	},

	Foundation:         mainNetFoundation,
	ElaAssetId:         elaAssetId,
	GenesisBlock:       genesisBlock,
	PowLimit:           powLimit,
	PowLimitBits:       0x1f0008ff,
	TargetTimespan:     24 * time.Hour,  // 24 hours
	TargetTimePerBlock: 2 * time.Minute, // 2 minute
	AdjustmentFactor:   4,               // 25% less, 400% more
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
	ExchangeRate:       1,
	MinCrossChainTxFee: 10000,

	SpvParams: MainNetSpvParams,
}

// TestNetParams defines the network parameters for the test network.
var TestNetParams = config.Params{
	Name:        "testnet",
	Magic:       20180011,
	DefaultPort: 21608,

	SeedList: []string{
		"did-testnet-001.elastos.org",
		"did-testnet-002.elastos.org",
		"did-testnet-003.elastos.org",
		"did-testnet-004.elastos.org",
		"did-testnet-005.elastos.org",
	},

	Foundation:         testNetFoundation,
	ElaAssetId:         elaAssetId,
	GenesisBlock:       genesisBlock,
	PowLimit:           powLimit,
	PowLimitBits:       0x1e1da5ff,
	TargetTimespan:     10 * time.Second * 10, // 100 second
	TargetTimePerBlock: 10 * time.Second,      // 10 second
	AdjustmentFactor:   4,                     // 25% less, 400% more
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
	ExchangeRate:       1,
	MinCrossChainTxFee: 10000,

	SpvParams: TestNetSpvParams,
}

// RegNetParams defines the network parameters for the regression test network.
var RegNetParams = config.Params{
	Name: "regnet",

	Foundation:         testNetFoundation,
	ElaAssetId:         elaAssetId,
	GenesisBlock:       genesisBlock,
	PowLimit:           powLimit,
	PowLimitBits:       0x207fffff,
	TargetTimespan:     1 * time.Second * 10, // 10 second
	TargetTimePerBlock: 1 * time.Second,      // 1 second
	AdjustmentFactor:   4,                    // 25% less, 400% more
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
	ExchangeRate:       1,
	MinCrossChainTxFee: 10000,

	SpvParams: RegNetSpvParams,
}
