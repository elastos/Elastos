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
	// originIssuanceAmount is the origin issuance ELA amount.
	originIssuanceAmount = 3300 * 10000 * 100000000

	// inflationPerYear is the inflation amount per year.
	inflationPerYear = originIssuanceAmount * 4 / 100

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

	Foundation:   mainNetFoundation,
	GenesisBlock: GenesisBlock(&mainNetFoundation),
	OriginArbiters: []string{
		"0248df6705a909432be041e0baa25b8f648741018f70d1911f2ed28778db4b8fe4",
		"02771faf0f4d4235744b30972d5f2c470993920846c761e4d08889ecfdc061cddf",
		"0342196610e57d75ba3afa26e030092020aec56822104e465cba1d8f69f8d83c8e",
		"02fa3e0d14e0e93ca41c3c0f008679e417cf2adb6375dd4bbbee9ed8e8db606a56",
		"03ab3ecd1148b018d480224520917c6c3663a3631f198e3b25cf4c9c76786b7850",
	},
	CRCArbiters: []CRCArbitratorParams{
		//todo add CRC arbiters
	},
	PowLimit:           powLimit,
	PowLimitBits:       0x1f0008ff,
	TargetTimespan:     24 * time.Hour,  // 24 hours
	TargetTimePerBlock: 2 * time.Minute, // 2 minute
	AdjustmentFactor:   4,               // 25% less, 400% more
	RewardPerBlock:     rewardPerBlock(2 * time.Minute),
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
	HeightVersions: []uint32{
		0,
		88812,
		1008812, //fixme edit height later
		1108812, //fixme edit height later
	},
	DPOSStartHeight:   1008812, //fixme edit height later
	MaxInactiveRounds: 3,
	InactiveDuration:  10,
	InactivePenalty:   100 * 100000000,
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

	Foundation:   testNetFoundation,
	GenesisBlock: GenesisBlock(&testNetFoundation),
	OriginArbiters: []string{
		"03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
		"02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
		"03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
		"03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
		"02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448",
	},
	CRCArbiters: []CRCArbitratorParams{
		//todo add CRC arbiters
	},
	PowLimit:           powLimit,
	PowLimitBits:       0x1f0008ff,
	TargetTimespan:     24 * time.Hour,  // 24 hours
	TargetTimePerBlock: 2 * time.Minute, // 2 minute
	AdjustmentFactor:   4,               // 25% less, 400% more
	RewardPerBlock:     rewardPerBlock(2 * time.Minute),
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
	HeightVersions: []uint32{
		0,
		0,
		1008812, //fixme edit height later
		1108812, //fixme edit height later
	},
	DPOSStartHeight:   1008812, //fixme edit height later
	MaxInactiveRounds: 3,
	InactiveDuration:  10,
	InactivePenalty:   100 * 100000000,
}

// RegNetParams defines the network parameters for the regression test network.
var RegNetParams = Params{
	Name:               "regnet",
	Foundation:         testNetFoundation,
	GenesisBlock:       GenesisBlock(&testNetFoundation),
	PowLimit:           powLimit,
	PowLimitBits:       0x207fffff,
	TargetTimePerBlock: 1 * time.Second,  // 1 second
	TargetTimespan:     10 * time.Second, // 10 seconds
	AdjustmentFactor:   4,                // 25% less, 400% more
	RewardPerBlock:     rewardPerBlock(1 * time.Second),
	CoinbaseMaturity:   100,
	MinTransactionFee:  100,
	HeightVersions: []uint32{
		0,
		0,
		1008812, //fixme edit height later
		1108812, //fixme edit height later
	},
	DPOSStartHeight:   1008812, //fixme edit height later
	MaxInactiveRounds: 3,
	InactiveDuration:  10,
	InactivePenalty:   100 * 100000000,
}

// CRCArbitratorParam defines parameters about arbitrators consensus and direct connection
type CRCArbitratorParams struct {
	PublicKey  []byte
	NetAddress string
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

	// OriginArbiters defines the original arbiters producing the block.
	OriginArbiters []string

	// CRCArbiters defines the fixed CRC arbiters producing the block.
	CRCArbiters []CRCArbitratorParams

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

	// RewardPerBlock is the reward amount per block.
	RewardPerBlock common.Fixed64

	// CoinbaseMaturity is the number of blocks required before newly mined
	// coins (coinbase transactions) can be spent.
	CoinbaseMaturity uint32

	// Disable transaction filter supports, include bloom filter tx type filter
	// etc.
	DisableTxFilters bool

	// MinTransactionFee defines the minimum fee of a transaction.
	MinTransactionFee int64

	// Define height versions about block chain.
	HeightVersions []uint32

	// DPOSStartHeight indicates the height of DPOS consensus begins.
	DPOSStartHeight uint32

	// InactiveDuration defines the duration blocks to set a producer as
	// inactive.
	InactiveDuration uint32

	// MaxInactiveRounds defines the maximum inactive rounds before producer
	// takes penalty.
	MaxInactiveRounds uint32

	// InactivePenalty defines the penalty amount the producer takes.
	InactivePenalty common.Fixed64

	// InactiveEliminateCount defines arbitrators count should be eliminated
	InactiveEliminateCount uint32
}

func rewardPerBlock(targetTimePerBlock time.Duration) common.Fixed64 {
	blockGenerateInterval := int64(targetTimePerBlock / time.Second)
	generatedBlocksPerYear := 365 * 24 * 60 * 60 / blockGenerateInterval
	return common.Fixed64(float64(inflationPerYear) / float64(generatedBlocksPerYear))
}
