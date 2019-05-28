package config

import (
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
)

// These variables are the chain consensus parameters for each default
// network.
var (
	// zeroHash represents a hash with all '0' value.
	zeroHash = common.Uint256{}

	// elaAsset is the transaction that create and register the ELA coin.
	elaAsset = types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &payload.RegisterAsset{
			Asset: payload.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: common.Uint168{},
		},
		Attributes: []*types.Attribute{},
		Inputs:     []*types.Input{},
		Outputs:    []*types.Output{},
		Programs:   []*program.Program{},
	}

	// attrNonce represents the nonce attribute used in the genesis coinbase
	// transaction.
	attrNonce = types.NewAttribute(types.Nonce,
		[]byte{77, 101, 130, 33, 7, 252, 253, 82})

	// genesisTime indicates the time when ELA genesis block created.
	genesisTime, _ = time.Parse(time.RFC3339, "2017-12-22T10:00:00Z")

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

	// "8ZZLWQUDSbjWUn8sEdxEFJsZiRFpzg53rJ"
	mainNetCRCAddress = common.Uint168{
		0x12, 0xca, 0xa4, 0xb0, 0x0b, 0x86, 0x8b,
		0xe7, 0x92, 0xe2, 0x40, 0x1e, 0x97, 0x4e,
		0xcd, 0x5f, 0xcb, 0x1f, 0xd9, 0xab, 0x37,
	}

	// "8JJCdEjMRm6x2rVsSMesL5gmoq7ts4wHMo"
	testNetCRCAddress = common.Uint168{
		0x12, 0x23, 0x3d, 0xfb, 0x54, 0x88, 0xbb,
		0xa2, 0xe9, 0xfa, 0x9a, 0x11, 0xbb, 0x07,
		0xac, 0x10, 0xcd, 0x77, 0x29, 0x41, 0x22,
	}

	// ELAAssetID represents the asset ID of ELA coin.
	ELAAssetID = elaAsset.Hash()

	// DestructionAddress indicates the "ELANULLXXXXXXXXXXXXXXXXXXXXXYvs3rr"
	// destruction address.
	DestructionAddress = common.Uint168{
		0x21, 0x20, 0xfe, 0xe5, 0xd7, 0xeb, 0x3e,
		0x5c, 0x7d, 0x31, 0x97, 0xfe, 0xcf, 0x6c,
		0x0d, 0xe3, 0x0f, 0x88, 0x9a, 0xce, 0xf7,
	}
)

// DefaultParams defines the default network parameters.
var DefaultParams = Params{
	Magic:       2017001,
	DefaultPort: 20338,

	DNSSeeds: []string{
		"node-mainnet-005.elastos.org:20338",
		"node-mainnet-010.elastos.org:20338",
		"node-mainnet-015.elastos.org:20338",
		"node-mainnet-020.elastos.org:20338",
		"node-mainnet-025.elastos.org:20338",
	},

	Foundation:   mainNetFoundation,
	CRCAddress:   mainNetCRCAddress,
	GenesisBlock: GenesisBlock(&mainNetFoundation),

	DPoSMagic:       2019000,
	DPoSDefaultPort: 20339,
	OriginArbiters: []string{
		"0248df6705a909432be041e0baa25b8f648741018f70d1911f2ed28778db4b8fe4",
		"02771faf0f4d4235744b30972d5f2c470993920846c761e4d08889ecfdc061cddf",
		"0342196610e57d75ba3afa26e030092020aec56822104e465cba1d8f69f8d83c8e",
		"02fa3e0d14e0e93ca41c3c0f008679e417cf2adb6375dd4bbbee9ed8e8db606a56",
		"03ab3ecd1148b018d480224520917c6c3663a3631f198e3b25cf4c9c76786b7850",
	},
	CRCArbiters: []string{
		"02089d7e878171240ce0e3633d3ddc8b1128bc221f6b5f0d1551caa717c7493062",
		"0268214956b8421c0621d62cf2f0b20a02c2dc8c2cc89528aff9bd43b45ed34b9f",
		"03cce325c55057d2c8e3fb03fb5871794e73b85821e8d0f96a7e4510b4a922fad5",
		"02661637ae97c3af0580e1954ee80a7323973b256ca862cfcf01b4a18432670db4",
		"027d816821705e425415eb64a9704f25b4cd7eaca79616b0881fc92ac44ff8a46b",
		"02d4a8f5016ae22b1acdf8a2d72f6eb712932213804efd2ce30ca8d0b9b4295ac5",
		"029a4d8e4c99a1199f67a25d79724e14f8e6992a0c8b8acf102682bd8f500ce0c1",
		"02871b650700137defc5d34a11e56a4187f43e74bb078e147dd4048b8f3c81209f",
		"02fc66cba365f9957bcb2030e89a57fb3019c57ea057978756c1d46d40dfdd4df0",
		"03e3fe6124a4ea269224f5f43552250d627b4133cfd49d1f9e0283d0cd2fd209bc",
		"02b95b000f087a97e988c24331bf6769b4a75e4b7d5d2a38105092a3aa841be33b",
		"02a0aa9eac0e168f3474c2a0d04e50130833905740a5270e8a44d6c6e85cf6d98c",
	},
	PowLimit:                 powLimit,
	PowLimitBits:             0x1f0008ff,
	TargetTimespan:           24 * time.Hour,  // 24 hours
	TargetTimePerBlock:       2 * time.Minute, // 2 minute
	AdjustmentFactor:         4,               // 25% less, 400% more
	RewardPerBlock:           rewardPerBlock(2 * time.Minute),
	CoinbaseMaturity:         100,
	MinTransactionFee:        100,
	MinCrossChainTxFee:       10000,
	CheckAddressHeight:       88812,
	VoteStartHeight:          290000,
	CRCOnlyDPOSHeight:        343400,
	PublicDPOSHeight:         402680,
	ToleranceDuration:        5 * time.Second,
	MaxInactiveRounds:        720 * 2,
	InactivePenalty:          0, //there will be no penalty in this version
	EmergencyInactivePenalty: 0, //there will be no penalty in this version
	GeneralArbiters:          24,
	CandidateArbiters:        72,
	PreConnectOffset:         360,
}

// TestNet returns the network parameters for the test network.
func (p *Params) TestNet() *Params {
	copy := *p
	copy.Magic = 2018101
	copy.DefaultPort = 21338

	copy.DNSSeeds = []string{
		"node-testnet-002.elastos.org:21338",
		"node-testnet-003.elastos.org:21338",
		"node-testnet-004.elastos.org:21338",
	}

	copy.Foundation = testNetFoundation
	copy.CRCAddress = testNetCRCAddress
	copy.GenesisBlock = GenesisBlock(&testNetFoundation)
	copy.DPoSMagic = 2019100
	copy.DPoSDefaultPort = 21339
	copy.OriginArbiters = []string{
		"03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
		"02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
		"03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
		"03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
		"02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448",
	}
	copy.CRCArbiters = []string{
		"03e435ccd6073813917c2d841a0815d21301ec3286bc1412bb5b099178c68a10b6",
		"038a1829b4b2bee784a99bebabbfecfec53f33dadeeeff21b460f8b4fc7c2ca771",
		"02435df9a4728e6250283cfa8215f16b48948d71936c4600b3a5b1c6fde70503ae",
		"027d44ee7e7a6c6ff13a130d15b18c75a3b47494c3e54fcffe5f4b10e225351e09",
		"02ad972fbfce4aaa797425138e4f3b22bcfa765ffad88b8a5af0ab515161c0a365",
		"0373eeae2bac0f5f14373ca603fe2c9caa9c7a79c7793246cec415d005e2fe53c0",
		"03503011cc4e44b94f73ed2c76c73182a75b4863f23d1e7083025eead945a8e764",
		"0270b6880e7fab8d02bea7d22639d7b5e07279dd6477baa713dacf99bb1d65de69",
		"030eed9f9c1d70307beba52ddb72a24a02582c0ee626ec93ee1dcef2eb308852dd",
		"026bba43feb19ce5859ffcf0ce9dd8b9d625130b686221da8b445fa9b8f978d7b9",
		"02bf9e37b3db0cbe86acf76a76578c6b17b4146df101ec934a00045f7d201f06dd",
		"03111f1247c66755d369a8c8b3a736dfd5cf464ca6735b659533cbe1268cd102a9",
	}
	copy.CheckAddressHeight = 0
	copy.VoteStartHeight = 200000
	copy.CRCOnlyDPOSHeight = 246700
	copy.PublicDPOSHeight = 300000
	return &copy
}

// RegNet returns the network parameters for the test network.
func (p *Params) RegNet() *Params {
	copy := *p
	copy.Magic = 2018201
	copy.DefaultPort = 22338

	copy.DNSSeeds = []string{
		"node-regtest-102.eadd.co:22338",
		"node-regtest-103.eadd.co:22338",
		"node-regtest-104.eadd.co:22338",
	}

	copy.Foundation = testNetFoundation
	copy.CRCAddress = testNetCRCAddress
	copy.GenesisBlock = GenesisBlock(&testNetFoundation)
	copy.DPoSMagic = 2019200
	copy.DPoSDefaultPort = 22339
	copy.OriginArbiters = []string{
		"03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
		"02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
		"03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
		"03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
		"02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448",
	}
	copy.CRCArbiters = []string{
		"0306e3deefee78e0e25f88e98f1f3290ccea98f08dd3a890616755f1a066c4b9b8",
		"02b56a669d713db863c60171001a2eb155679cad186e9542486b93fa31ace78303",
		"0250c5019a00f8bb4fd59bb6d613c70a39bb3026b87cfa247fd26f59fd04987855",
		"02e00112e3e9defe0f38f33aaa55551c8fcad6aea79ab2b0f1ec41517fdd05950a",
		"020aa2d111866b59c70c5acc60110ef81208dcdc6f17f570e90d5c65b83349134f",
		"03cd41a8ed6104c1170332b02810237713369d0934282ca9885948960ae483a06d",
		"02939f638f3923e6d990a70a2126590d5b31a825a0f506958b99e0a42b731670ca",
		"032ade27506951c25127b0d2cb61d164e0bad8aec3f9c2e6785725a6ab6f4ad493",
		"03f716b21d7ae9c62789a5d48aefb16ba1e797b04a2ec1424cd6d3e2e0b43db8cb",
		"03488b0aace5fe5ee5a1564555819074b96cee1db5e7be1d74625240ef82ddd295",
		"03c559769d5f7bb64c28f11760cb36a2933596ca8a966bc36a09d50c24c48cc3e8",
		"03b5d90257ad24caf22fa8a11ce270ea57f3c2597e52322b453d4919ebec4e6300",
	}

	copy.CheckAddressHeight = 0
	copy.VoteStartHeight = 170000
	copy.CRCOnlyDPOSHeight = 211000
	copy.PublicDPOSHeight = 234000
	return &copy
}

// InstantBlock returns the network parameters for generate instant block.
func (p *Params) InstantBlock() *Params {
	copy := *p
	copy.PowLimitBits = 0x207fffff
	copy.TargetTimespan = 10 * time.Second
	copy.TargetTimePerBlock = 1 * time.Second
	return &copy
}

type Params struct {
	// Magic defines the magic number of the peer-to-peer network.
	Magic uint32

	// DefaultPort defines the default peer-to-peer port for the network.
	DefaultPort uint16

	// DNSSeeds defines a list of DNS seeds for the network to discover peers.
	DNSSeeds []string

	// The interface/port to listen for connections.
	ListenAddrs []string

	// Foundation defines the foundation address which receiving mining
	// rewards.
	Foundation common.Uint168

	// CRCAddress defines the CRC address which receiving mining rewards.
	CRCAddress common.Uint168

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

	// RewardPerBlock is the reward amount per block.
	RewardPerBlock common.Fixed64

	// CoinbaseMaturity is the number of blocks required before newly mined
	// coins (coinbase transactions) can be spent.
	CoinbaseMaturity uint32

	// Disable transaction filter supports, include bloom filter tx type filter
	// etc.
	DisableTxFilters bool

	// MinTransactionFee defines the minimum fee of a transaction.
	MinTransactionFee common.Fixed64

	// MinCrossChainTxFee defines the min fee of cross chain transaction
	MinCrossChainTxFee common.Fixed64

	// OriginArbiters defines the original arbiters producing the block.
	OriginArbiters []string

	// CheckAddressHeight defines the height begin to check output hash.
	CheckAddressHeight uint32

	// VoteStartHeight indicates the height of starting register producer and
	// vote related.
	VoteStartHeight uint32

	// CRCOnlyDPOSHeight (H1) indicates the height of DPOS consensus begins with
	// only CRC producers participate in producing blocks.
	CRCOnlyDPOSHeight uint32

	// PublicDPOSHeight (H2) indicates the height when public registered and
	// elected producers participate in DPOS consensus.
	PublicDPOSHeight uint32

	// CRCArbiters defines the fixed CRC arbiters producing the block.
	CRCArbiters []string

	// DPoSMagic defines the magic number used in the DPoS network.
	DPoSMagic uint32

	// DPoSDefaultPort defines the default port for the DPoS network.
	DPoSDefaultPort uint16

	// PreConnectOffset defines the offset blocks to pre-connect to the block
	// producers.
	PreConnectOffset uint32

	// GeneralArbiters defines the number of general(no-CRC) arbiters.
	GeneralArbiters int

	// CandidateArbiters defines the number of needed candidate arbiters.
	CandidateArbiters int

	// ToleranceDuration defines the tolerance duration of the DPoS consensus.
	ToleranceDuration time.Duration

	// MaxInactiveRounds defines the maximum inactive rounds before producer
	// takes penalty.
	MaxInactiveRounds uint32

	// InactivePenalty defines the penalty amount the producer takes.
	InactivePenalty common.Fixed64

	// EmergencyInactivePenalty defines the penalty amount the emergency
	// producer takes.
	EmergencyInactivePenalty common.Fixed64
}

// rewardPerBlock calculates the reward for each block by a specified time
// duration.
func rewardPerBlock(targetTimePerBlock time.Duration) common.Fixed64 {
	blockGenerateInterval := int64(targetTimePerBlock / time.Second)
	generatedBlocksPerYear := 365 * 24 * 60 * 60 / blockGenerateInterval
	return common.Fixed64(float64(inflationPerYear) / float64(generatedBlocksPerYear))
}

// GenesisBlock creates a genesis block by the specified foundation address.
// The genesis block goes different because the foundation address in each
// network is different.
func GenesisBlock(foundation *common.Uint168) *types.Block {
	coinBase := types.Transaction{
		Version:        0,
		TxType:         types.CoinBase,
		PayloadVersion: payload.CoinBaseVersion,
		Payload:        &payload.CoinBase{},
		Attributes:     []*types.Attribute{&attrNonce},
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  zeroHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Outputs: []*types.Output{
			{
				AssetID:     ELAAssetID,
				Value:       3300 * 10000 * 100000000,
				ProgramHash: *foundation,
			},
		},
		LockTime: 0,
		Programs: []*program.Program{},
	}

	merkleRoot, _ := crypto.ComputeRoot([]common.Uint256{coinBase.Hash(),
		ELAAssetID})

	return &types.Block{
		Header: types.Header{
			Version:    0,
			Previous:   zeroHash,
			MerkleRoot: merkleRoot,
			Timestamp:  uint32(genesisTime.Unix()),
			Bits:       0x1d03ffff,
			Nonce:      2083236893,
			Height:     0,
		},
		Transactions: []*types.Transaction{&coinBase, &elaAsset},
	}
}
