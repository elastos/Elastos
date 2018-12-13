package spv

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"log"
	"math/big"
	"os"
	"time"
)

const (
	DefaultConfigFilename = "./config.json"
)

var (
	Parameters configParams
	Version    string
	mainNet    = &ChainParams{
		Name:               "MainNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x1f0008ff,
		TargetTimespan:     time.Second * 60 * 2 * 720,
		TargetTimePerBlock: time.Second * 60 * 2,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		SpendCoinbaseSpan:  100,
	}
	testNet = &ChainParams{
		Name:               "TestNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x1e1da5ff,
		TargetTimespan:     time.Second * 10 * 10,
		TargetTimePerBlock: time.Second * 10,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		SpendCoinbaseSpan:  100,
	}
	regNet = &ChainParams{
		Name:               "RegNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x207fffff,
		TargetTimespan:     time.Second * 1 * 10,
		TargetTimePerBlock: time.Second * 1,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		SpendCoinbaseSpan:  100,
	}
)

type PowConfiguration struct {
	PayToAddr        string `json:"PayToAddr"`
	MiningServerIP   string `josn:"MiningServerIP"`
	MiningServerPort int    `josn:"MiningServerPort"`
	MiningSelfPort   int    `josn:"MiningSelfPort"`
	TestNet          bool   `json:"testnet"`
	AutoMining       bool   `json:"AutoMining"`
	MinerInfo        string `json:"MinerInfo"`
	MinTxFee         int    `json:"MinTxFee"`
	ActiveNet        string `json:"ActiveNet"`
}

type Configuration struct {
	Magic                      uint32           `json:"Magic"`
	SpvMagic                   uint32           `json:"SpvMagic"`
	Version                    int              `json:"Version"`
	SeedList                   []string         `json:"SeedList"`
	SpvSeedList                []string         `json:"SpvSeedList"`
	SpvMinOutbound             int              `json:"SpvMinOutbound"`
	SpvMaxConnections          int              `json:"SpvMaxConnections"`
	SpvPrintLevel              int              `json:"SpvPrintLevel"`
	ExchangeRate               float64          `json:"ExchangeRate"`
	MinCrossChainTxFee         int              `json:"MinCrossChainTxFee"`
	HttpRestPort               int              `json:"HttpRestPort"`
	RestCertPath               string           `json:"RestCertPath"`
	RestKeyPath                string           `json:"RestKeyPath"`
	HttpInfoPort               uint16           `json:"HttpInfoPort"`
	HttpInfoStart              bool             `json:"HttpInfoStart"`
	OpenService                bool             `json:"OpenService"`
	HttpWsPort                 int              `json:"HttpWsPort"`
	WsHeartbeatInterval        time.Duration    `json:"WsHeartbeatInterval"`
	HttpJsonPort               int              `json:"HttpJsonPort"`
	OauthServerUrl             string           `json:"OauthServerUrl"`
	NoticeServerUrl            string           `json:"NoticeServerUrl"`
	NodePort                   uint16           `json:"NodePort"`
	WebSocketPort              int              `json:"WebSocketPort"`
	PrintLevel                 int              `json:"PrintLevel"`
	IsTLS                      bool             `json:"IsTLS"`
	CertPath                   string           `json:"CertPath"`
	KeyPath                    string           `json:"KeyPath"`
	CAPath                     string           `json:"CAPath"`
	MultiCoreNum               uint             `json:"MultiCoreNum"`
	MaxLogsSize                int64            `json:"MaxLogsSize"`
	MaxPerLogSize              int64            `json:"MaxPerLogSize"`
	MaxTxInBlock               int              `json:"MaxTransactionInBlock"`
	MaxBlockSize               int              `json:"MaxBlockSize"`
	PowConfiguration           PowConfiguration `json:"PowConfiguration"`
	FoundationAddress          string           `json:"FoundationAddress"`
	MainChainFoundationAddress string           `json:"MainChainFoundationAddress"`
}

type ConfigFile struct {
	ConfigFile Configuration `json:"Configuration"`
}

type ChainParams struct {
	Name               string
	PowLimit           *big.Int
	PowLimitBits       uint32
	TargetTimespan     time.Duration
	TargetTimePerBlock time.Duration
	AdjustmentFactor   int64
	MaxOrphanBlocks    int
	MinMemoryNodes     uint32
	SpendCoinbaseSpan  uint32
}

type configParams struct {
	*Configuration
	ChainParam *ChainParams
}

func init() {
	config := ConfigFile{}
	file, e := ioutil.ReadFile(DefaultConfigFilename)
	if e == nil {
		// Remove the UTF-8 Byte Order Mark
		file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

		e = json.Unmarshal(file, &config)
		if e != nil {
			log.Fatalf("Unmarshal json file error %v", e)
			os.Exit(1)
		}
	} else {
		config.ConfigFile.Magic = 20181206
		config.ConfigFile.SpvMagic = 20180627
		config.ConfigFile.SpvSeedList = []string{"node-regtest-002.elastos.org:22866", "node-regtest-003.elastos.org:22866", "node-regtest-004.elastos.org:22866"}
		config.ConfigFile.NodePort = 21608
		config.ConfigFile.Version = 23
		config.ConfigFile.PowConfiguration.ActiveNet = "MainNet"
		config.ConfigFile.PowConfiguration.PayToAddr = "ELVzVKcEYyV1B4YpdWDJFUcfiAYynD4Cpb"
		config.ConfigFile.PowConfiguration.AutoMining = false
		config.ConfigFile.PowConfiguration.MinerInfo = "Geth"
		config.ConfigFile.PowConfiguration.MinTxFee = 100
		config.ConfigFile.MinCrossChainTxFee = 10000
		config.ConfigFile.ExchangeRate = 1.0
		config.ConfigFile.SpvPrintLevel = 1
		config.ConfigFile.MaxPerLogSize = 0
		config.ConfigFile.MaxLogsSize = 0
		config.ConfigFile.MainChainFoundationAddress = "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3"
	}
	//	Parameters = &(config.ConfigFile)
	Parameters.Configuration = &(config.ConfigFile)
	if Parameters.PowConfiguration.ActiveNet == "MainNet" {
		Parameters.ChainParam = mainNet
	} else if Parameters.PowConfiguration.ActiveNet == "TestNet" {
		Parameters.ChainParam = testNet
	} else if Parameters.PowConfiguration.ActiveNet == "RegNet" {
		Parameters.ChainParam = regNet
	}
}
