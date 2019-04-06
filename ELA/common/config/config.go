package config

import (
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA/common"
)

var (
	Parameters ConfigParams

	MainNet = ChainParams{
		Name:               "MainNet",
		PowLimit:           powLimit,
		PowLimitBits:       0x1f0008ff,
		TargetTimePerBlock: time.Minute * 2,
		TargetTimespan:     time.Minute * 2 * 720,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
		RewardPerBlock:     rewardPerBlock(time.Minute * 2),
	}

	TestNet = ChainParams{
		Name:               "TestNet",
		PowLimit:           powLimit,
		PowLimitBits:       0x1f0008ff,
		TargetTimePerBlock: time.Minute * 2,
		TargetTimespan:     time.Minute * 2 * 720,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
		RewardPerBlock:     rewardPerBlock(time.Minute * 2),
	}

	RegNet = ChainParams{
		Name:               "RegNet",
		PowLimit:           powLimit,
		PowLimitBits:       0x1f0008ff,
		TargetTimePerBlock: time.Minute * 2,
		TargetTimespan:     time.Second * 2 * 720,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
		RewardPerBlock:     rewardPerBlock(time.Minute * 2),
	}
)

type PowConfiguration struct {
	PayToAddr    string `json:"PayToAddr"`
	AutoMining   bool   `json:"AutoMining"`
	MinerInfo    string `json:"MinerInfo"`
	MinTxFee     int    `json:"MinTxFee"`
	InstantBlock bool   `json:"InstantBlock"`
}

type RpcConfiguration struct {
	User        string   `json:"User"`
	Pass        string   `json:"Pass"`
	WhiteIPList []string `json:"WhiteIPList"`
}

// CRCArbitratorParam defines parameters about arbitrators consensus and direct connection
type CRCArbiter struct {
	PublicKey  string `json:"PublicKey"`
	NetAddress string `json:"NetAddress"`
}

type Configuration struct {
	ActiveNet            string               `json:"ActiveNet"`
	Magic                uint32               `json:"Magic"`
	Version              int                  `json:"Version"`
	SeedList             []string             `json:"SeedList"`
	HttpInfoPort         uint16               `json:"HttpInfoPort"`
	HttpInfoStart        bool                 `json:"HttpInfoStart"`
	HttpRestPort         int                  `json:"HttpRestPort"`
	HttpWsPort           int                  `json:"HttpWsPort"`
	HttpJsonPort         int                  `json:"HttpJsonPort"`
	NodePort             uint16               `json:"NodePort"`
	PrintLevel           uint8                `json:"PrintLevel"`
	MaxLogsSize          int64                `json:"MaxLogsSize"`
	MaxPerLogSize        int64                `json:"MaxPerLogSize"`
	RestCertPath         string               `json:"RestCertPath"`
	RestKeyPath          string               `json:"RestKeyPath"`
	MinCrossChainTxFee   int                  `json:"MinCrossChainTxFee"`
	FoundationAddress    string               `json:"FoundationAddress"`
	CRCAddress           string               `json:"CRCAddress"`
	PowConfiguration     PowConfiguration     `json:"PowConfiguration"`
	RpcConfiguration     RpcConfiguration     `json:"RpcConfiguration"`
	EnableArbiter        bool                 `json:"EnableArbiter"`
	ArbiterConfiguration ArbiterConfiguration `json:"ArbiterConfiguration"`
	CheckAddressHeight   uint32               `json:"CheckAddressHeight"`
	VoteStartHeight      uint32               `json:"VoteStartHeight"`
	CRCOnlyDPOSHeight    uint32               `json:"CRCOnlyDPOSHeight"`
	PublicDPOSHeight     uint32               `json:"PublicDPOSHeight"`
}

type ArbiterConfiguration struct {
	Magic                    uint32         `json:"Magic"`
	IPAddress                string         `json:"IPAddress"`
	NodePort                 uint16         `json:"NodePort"`
	PrintLevel               uint8          `json:"PrintLevel"`
	SignTolerance            uint64         `json:"SignTolerance"`
	MaxLogsSize              int64          `json:"MaxLogsSize"`
	MaxPerLogSize            int64          `json:"MaxPerLogSize"`
	OriginArbiters           []string       `json:"OriginArbiters"`
	CRCArbiters              []CRCArbiter   `json:"CRCArbiters"`
	NormalArbitratorsCount   int            `json:"NormalArbitratorsCount"`
	CandidatesCount          int            `json:"CandidatesCount"`
	EmergencyInactivePenalty common.Fixed64 `json:"EmergencyInactivePenalty"`
	MaxInactiveRounds        uint32         `json:"MaxInactiveRounds"`
	InactivePenalty          common.Fixed64 `json:"InactivePenalty"`
	InactiveEliminateCount   uint32         `json:"InactiveEliminateCount"`
	EnableEventRecord        bool           `json:"EnableEventRecord"`
	PreConnectOffset         uint32         `json:"PreConnectOffset"`
}

type Seed struct {
	PublicKey string `json:"PublicKey"`
	Address   string `json:"Address"`
}

type ConfigFile struct {
	Configuration `json:"Configuration"`
}

type ChainParams struct {
	Name               string
	PowLimit           *big.Int
	PowLimitBits       uint32
	TargetTimePerBlock time.Duration
	TargetTimespan     time.Duration
	AdjustmentFactor   int64
	MaxOrphanBlocks    int
	MinMemoryNodes     uint32
	CoinbaseLockTime   uint32
	RewardPerBlock     common.Fixed64
}

type ConfigParams struct {
	*Configuration
	ChainParam *ChainParams
}
