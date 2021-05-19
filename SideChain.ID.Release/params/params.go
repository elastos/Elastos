package params

import (
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/auxpow"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	ela "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// These variables are the chain consensus parameters.
var (
	// elaAsset is the transaction that create and register the ELA coin.
	elaAsset = types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &types.PayloadRegisterAsset{
			Asset: types.Asset{
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
		Programs:   []*types.Program{},
	}

	// genesisTime indicates the time when ELA genesis block created.
	genesisTime, _ = time.Parse(time.RFC3339, "2018-06-30T12:00:00Z")

	// genesisHeader represent the block header of the genesis block.
	genesisHeader = types.Header{
		Base: types.BaseHeader{
			Version:    types.BlockVersion,
			Previous:   common.Uint256{},
			MerkleRoot: ElaAssetId,
			Timestamp:  uint32(genesisTime.Unix()),
			Bits:       0x1d03ffff,
			Nonce:      types.GenesisNonce,
			Height:     uint32(0),
		},
		SideAuxPow: auxpow.SideAuxPow{
			SideAuxBlockTx: ela.Transaction{
				TxType:         ela.SideChainPow,
				PayloadVersion: payload.SideChainPowVersion,
				Payload:        new(payload.SideChainPow),
			},
		},
	}

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

	// ELAAssetID represents the asset ID of ELA coin.
	ElaAssetId = elaAsset.Hash()

	// GenesisBlock represent the genesis block of the ID chain.
	GenesisBlock = &types.Block{
		Header:       &genesisHeader,
		Transactions: []*types.Transaction{&elaAsset},
	}
)

// MainNetParams defines the network parameters for the main network.
var MainNetParams = config.Params{
	Name:        "mainnet",
	Magic:       2017002,
	DefaultPort: 20608,

	DNSSeeds: []string{
		"node-mainnet-005.elastos.org:20608",
		"node-mainnet-010.elastos.org:20608",
		"node-mainnet-015.elastos.org:20608",
		"node-mainnet-020.elastos.org:20608",
		"node-mainnet-025.elastos.org:20608",
	},

	Foundation:           mainNetFoundation,
	ElaAssetId:           ElaAssetId,
	GenesisBlock:         GenesisBlock,
	PowLimit:             powLimit,
	PowLimitBits:         0x1f0008ff,
	TargetTimespan:       24 * time.Hour,  // 24 hours
	TargetTimePerBlock:   2 * time.Minute, // 2 minute
	AdjustmentFactor:     4,               // 25% less, 400% more
	CoinbaseMaturity:     100,
	MinTransactionFee:    100,
	ExchangeRate:         1,
	MinCrossChainTxFee:   10000,
	CheckPowHeaderHeight: 160340,
}

// TestNetParams defines the network parameters for the test network.
var TestNetParams = testNetParams(MainNetParams)

// RegNetParams defines the network parameters for the regression network.
var RegNetParams = regNetParams(MainNetParams)

// testNetParams returns the network parameters for the test network.
func testNetParams(cfg config.Params) config.Params {
	cfg.Name = "testnet"
	cfg.Magic = 2018102
	cfg.DefaultPort = 21608
	cfg.DNSSeeds = []string{
		"node-testnet-002.elastos.org:21608",
		"node-testnet-003.elastos.org:21608",
		"node-testnet-004.elastos.org:21608",
	}
	cfg.Foundation = testNetFoundation
	cfg.CheckPowHeaderHeight = 100000
	return cfg
}

// regNetParams returns the network parameters for the regression network.
func regNetParams(cfg config.Params) config.Params {
	cfg.Name = "regnet"
	cfg.Magic = 2018202
	cfg.DefaultPort = 22608
	cfg.DNSSeeds = []string{
		"node-regtest-102.eadd.co:22608",
		"node-regtest-103.eadd.co:22608",
		"node-regtest-104.eadd.co:22608",
	}
	cfg.Foundation = testNetFoundation
	cfg.CheckPowHeaderHeight = 42800
	return cfg
}

// InstantBlock changes the given network parameter to instant block mode.
func InstantBlock(cfg *config.Params) {
	cfg.PowLimitBits = 0x207fffff
	cfg.TargetTimespan = 1 * time.Second * 10
	cfg.TargetTimePerBlock = 1 * time.Second
}
