package params

import (
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/config"

	"github.com/elastos/Elastos.ELA/common"
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
var MainNetSpvParams = config.Params{
	Magic:      2017001,
	Foundation: mainNetFoundation,

	DNSSeeds: []string{
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
var TestNetSpvParams = config.Params{
	Magic:      2018001,
	Foundation: mainNetFoundation,

	DNSSeeds: []string{
		"node-testnet-001.elastos.org",
		"node-testnet-002.elastos.org",
		"node-testnet-003.elastos.org",
		"node-testnet-004.elastos.org",
		"node-testnet-005.elastos.org",
	},

	DefaultPort: 21866,
}

// RegNetSpvParams defines the network parameters for the regression network SPV.
var RegNetSpvParams = config.Params{
	Foundation: mainNetFoundation,
}

// MainNetParams defines the network parameters for the main network.
var MainNetParams = config.Params{
	Name:        "mainnet",
	Magic:       2019005,
	DefaultPort: 20628,

	DNSSeeds: []string{
		"node-mainnet-005.elastos.org:20628",
		"node-mainnet-010.elastos.org:20628",
		"node-mainnet-015.elastos.org:20628",
		"node-mainnet-020.elastos.org:20628",
		"node-mainnet-025.elastos.org:20628",
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
	CheckPowHeaderHeight: 31538,
}

// TestNetParams defines the network parameters for the test network.
var TestNetParams = testNetParams(MainNetParams)

// RegNetParams defines the network parameters for the regression network.
var RegNetParams = regNetParams(MainNetParams)

// testNetParams returns the network parameters for the test network.
func testNetParams(cfg config.Params) config.Params {
	cfg.Name = "testnet"
	cfg.Magic = 2019105
	cfg.DefaultPort = 21628
	cfg.DNSSeeds = []string{
		"node-testnet-002.elastos.org:21628",
		"node-testnet-003.elastos.org:21628",
		"node-testnet-004.elastos.org:21628",
	}
	cfg.Foundation = testNetFoundation
	cfg.CheckPowHeaderHeight = 30000
	return cfg
}

// regNetParams returns the network parameters for the regression network.
func regNetParams(cfg config.Params) config.Params {
	cfg.Name = "regnet"
	cfg.Magic = 2019205
	cfg.DefaultPort = 22628
	cfg.DNSSeeds = []string{
		"node-regtest-102.eadd.co:22628",
		"node-regtest-103.eadd.co:22628",
		"node-regtest-104.eadd.co:22628",
	}
	cfg.Foundation = testNetFoundation
	cfg.CheckPowHeaderHeight = 20000
	return cfg
}

// InstantBlock changes the given network parameter to instant block mode.
func InstantBlock(cfg *config.Params) {
	cfg.PowLimitBits = 0x207fffff
	cfg.TargetTimespan = 1 * time.Second * 10
	cfg.TargetTimePerBlock = 1 * time.Second
}