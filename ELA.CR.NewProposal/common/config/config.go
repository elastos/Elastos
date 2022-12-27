// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package config

import (
	"time"

	"github.com/elastos/Elastos.ELA/common"
)

var (
	Parameters *Configuration
)

// PowConfiguration defines the Proof-of-Work parameters.
type PowConfiguration struct {
	PayToAddr    string `json:"PayToAddr"`
	AutoMining   bool   `json:"AutoMining"`
	MinerInfo    string `json:"MinerInfo"`
	MinTxFee     int    `json:"MinTxFee"`
	InstantBlock bool   `json:"InstantBlock"`
}

// RpcConfiguration defines the JSON-RPC authenticate parameters.
type RpcConfiguration struct {
	User        string   `json:"User"`
	Pass        string   `json:"Pass"`
	WhiteIPList []string `json:"WhiteIPList"`
}

// Configuration defines the configurable parameters to run a ELA node.
type Configuration struct {
	ActiveNet                   string            `json:"ActiveNet"`
	Magic                       uint32            `json:"Magic"`
	DNSSeeds                    []string          `json:"DNSSeeds"`
	DisableDNS                  bool              `json:"DisableDNS"`
	PermanentPeers              []string          `json:"PermanentPeers"`
	HttpInfoPort                uint16            `json:"HttpInfoPort"`
	HttpInfoStart               bool              `json:"HttpInfoStart"`
	HttpRestPort                int               `json:"HttpRestPort"`
	HttpRestStart               bool              `json:"HttpRestStart"`
	HttpWsPort                  int               `json:"HttpWsPort"`
	HttpWsStart                 bool              `json:"HttpWsStart"`
	HttpJsonPort                int               `json:"HttpJsonPort"`
	EnableRPC                   bool              `json:"EnableRPC"`
	NodePort                    uint16            `json:"NodePort"`
	PrintLevel                  uint32            `json:"PrintLevel"`
	MaxLogsSize                 int64             `json:"MaxLogsSize"`
	MaxPerLogSize               int64             `json:"MaxPerLogSize"`
	RestCertPath                string            `json:"RestCertPath"`
	RestKeyPath                 string            `json:"RestKeyPath"`
	MinCrossChainTxFee          common.Fixed64    `json:"MinCrossChainTxFee"`
	FoundationAddress           string            `json:"FoundationAddress"`
	PowConfiguration            PowConfiguration  `json:"PowConfiguration"`
	RpcConfiguration            RpcConfiguration  `json:"RpcConfiguration"`
	DPoSConfiguration           DPoSConfiguration `json:"DPoSConfiguration"`
	CRConfiguration             CRConfiguration   `json:"CRConfiguration"`
	CheckAddressHeight          uint32            `json:"CheckAddressHeight"`
	VoteStartHeight             uint32            `json:"VoteStartHeight"`
	CRCOnlyDPOSHeight           uint32            `json:"CRCOnlyDPOSHeight"`
	PublicDPOSHeight            uint32            `json:"PublicDPOSHeight"`
	EnableActivateIllegalHeight uint32            `json:"EnableActivateIllegalHeight"`
	CheckRewardHeight           uint32            `json:"CheckRewardHeight"`
	VoteStatisticsHeight        uint32            `json:"VoteStatisticsHeight"`
	ProfilePort                 uint32            `json:"ProfilePort"`
	MaxBlockSize                uint32            `json:"MaxBlockSize"`
	MaxBlockHeaderSize          uint32            `json:"MaxBlockHeaderSize"`
	MaxTxPerBlock               uint32            `json:"MaxTxPerBlock"`
	EnableHistory               bool              `json:"EnableHistory"`
	HistoryStartHeight          uint32            `json:"HistoryStartHeight"`
	EnableUtxoDB                bool              `json:"EnableUtxoDB"`
	WalletPath                  string            `json:"WalletPath"`
	RPCServiceLevel             string            `json:"RPCServiceLevel"`
	NodeProfileStrategy         string            `json:"NodeProfileStrategy"`
	TxCacheVolume               uint32            `json:"TxCacheVolume"`
	MaxNodePerHost              uint32            `json:"MaxNodePerHost"`
}

// DPoSConfiguration defines the DPoS consensus parameters.
type DPoSConfiguration struct {
	EnableArbiter            bool           `json:"EnableArbiter"`
	Magic                    uint32         `json:"Magic"`
	IPAddress                string         `json:"IPAddress"`
	DPoSPort                 uint16         `json:"DPoSPort"`
	SignTolerance            time.Duration  `json:"SignTolerance"`
	OriginArbiters           []string       `json:"OriginArbiters"`
	CRCArbiters              []string       `json:"CRCArbiters"`
	NormalArbitratorsCount   int            `json:"NormalArbitratorsCount"`
	CandidatesCount          int            `json:"CandidatesCount"`
	EmergencyInactivePenalty common.Fixed64 `json:"EmergencyInactivePenalty"`
	MaxInactiveRounds        uint32         `json:"MaxInactiveRounds"`
	InactivePenalty          common.Fixed64 `json:"InactivePenalty"`
	PreConnectOffset         uint32         `json:"PreConnectOffset"`
}

type CRConfiguration struct {
	MemberCount                        uint32         `json:"MemberCount"`
	VotingPeriod                       uint32         `json:"VotingPeriod"`
	DutyPeriod                         uint32         `json:"DutyPeriod"`
	DepositLockupBlocks                uint32         `json:"DepositLockupBlocks"`
	ProposalCRVotingPeriod             uint32         `json:"ProposalCRVotingPeriod"`
	ProposalPublicVotingPeriod         uint32         `json:"ProposalPublicVotingPeriod"`
	CRAgreementCount                   uint32         `json:"CRAgreementCount"`
	VoterRejectPercentage              float64        `json:"VoterRejectPercentage"`
	CRCAppropriatePercentage           float64        `json:"CRCAppropriatePercentage"`
	MaxCommitteeProposalCount          uint32         `json:"MaxCommitteeProposalCount"`
	SecretaryGeneral                   string         `json:"SecretaryGeneral"`
	MaxProposalTrackingCount           uint8          `json:"MaxProposalTrackingCount"`
	RegisterCRByDIDHeight              uint32         `json:"RegisterCRByDIDHeight"`
	MaxCRAssetsAddressUTXOCount        uint32         `json:"MaxCRAssetsAddressUTXOCount"`
	MinCRAssetsAddressUTXOCount        uint32         `json:"MinCRAssetsAddressUTXOCount"`
	CRAssetsRectifyTransactionHeight   uint32         `json:"CRAssetsRectifyTransactionHeight"`
	CRCProposalWithdrawPayloadV1Height uint32         `json:"CRCProposalWithdrawPayloadV1Height"`
	CRCAddress                         string         `json:"CRCAddress"`
	CRAssetsAddress                    string         `json:"CRAssetsAddress"`
	CRExpensesAddress                  string         `json:"CRExpensesAddress"`
	CRVotingStartHeight                uint32         `json:"CRVotingStartHeight"`
	CRCommitteeStartHeight             uint32         `json:"CRCommitteeStartHeight"`
	RectifyTxFee                       common.Fixed64 `json:"RectifyTxFee"`
	RealWithdrawSingleFee              common.Fixed64 `json:"RealWithdrawSingleFee"`
}

type RPCServiceLevel byte

const (
	// Allowed  query transaction, and configuration related options.
	ConfigurationPermitted RPCServiceLevel = iota

	// Allowed mining from RPC.
	MiningPermitted

	// Allowed query and transaction (
	//	such as sendrawtransaction) related options.
	TransactionPermitted

	// Allowed using wallet related function.
	WalletPermitted

	// Allowed only query related options.
	QueryOnly
)

func (l RPCServiceLevel) String() string {
	switch l {
	case ConfigurationPermitted:
		return "ConfigurationPermitted"
	case MiningPermitted:
		return "MiningPermitted"
	case TransactionPermitted:
		return "TransactionPermitted"
	case WalletPermitted:
		return "WalletPermitted"
	case QueryOnly:
		return "QueryOnly"
	default:
		return "Unknown"
	}
}

func RPCServiceLevelFromString(str string) RPCServiceLevel {
	switch str {
	case "ConfigurationPermitted":
		return ConfigurationPermitted
	case "MiningPermitted":
		return MiningPermitted
	case "TransactionPermitted":
		return TransactionPermitted
	case "WalletPermitted":
		return WalletPermitted
	case "QueryOnly":
		return QueryOnly
	default:
		return ConfigurationPermitted
	}
}

type NodeProfileStrategy byte

const (
	// Node will balance usage of CPU and memory.
	Balanced NodeProfileStrategy = iota

	// Node will optimise the block processing procedure, super node strongly
	//	recommended.
	SpeedFirst

	// Node will optimise the usage of memory usage, note this may slow down
	//	block processing, do no use this if your memory is extremely low (
	//	specifically small than 2G bytes).
	MemoryFirst
)

func (s NodeProfileStrategy) String() string {
	switch s {
	case Balanced:
		return "Balanced"
	case SpeedFirst:
		return "SpeedFirst"
	case MemoryFirst:
		return "MemoryFirst"
	default:
		return "Unknown"
	}
}
