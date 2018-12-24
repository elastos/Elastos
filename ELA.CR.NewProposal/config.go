package main

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"strings"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
)

const (
	ConfigFilename = "./config.json"
)

var (
	activeNetParams = &config.MainNetParams

	cfg = loadConfigParams()
)

func loadConfigFile() *config.Configuration {
	file, err := ioutil.ReadFile(ConfigFilename)
	if err != nil {
		return &configTemplate
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	var cfgFile config.ConfigFile
	if err := json.Unmarshal(file, &cfgFile); err != nil {
		return &configTemplate
	}

	return &cfgFile.Configuration
}

func loadConfigParams() *config.ConfigParams {
	cfg := loadConfigFile()

	var chainParams config.ChainParams
	switch strings.ToLower(cfg.PowConfiguration.ActiveNet) {
	case "mainnet", "main":
		chainParams = config.MainNet
		activeNetParams = &config.MainNetParams

	case "testnet", "test":
		chainParams = config.TestNet
		activeNetParams = &config.TestNetParams

	case "regnet", "reg":
		chainParams = config.RegNet
		activeNetParams = &config.RegNetParams
	}

	config.Parameters = config.ConfigParams{
		Configuration: cfg,
		ChainParam:    &chainParams,
	}

	activeNetParams.Magic = cfg.Magic
	activeNetParams.DefaultPort = cfg.NodePort
	activeNetParams.SeedList = cfg.SeedList
	foundation, err := common.Uint168FromAddress(cfg.FoundationAddress)
	if err == nil {
		blockchain.FoundationAddress = *foundation
		activeNetParams.Foundation = *foundation
		activeNetParams.GenesisBlock = config.GenesisBlock(*foundation)
	}
	if len(cfg.ArbiterConfiguration.OriginArbiters) > 0 {
		activeNetParams.OriginArbiters = cfg.ArbiterConfiguration.OriginArbiters
	}

	return &config.Parameters
}

var configTemplate = config.Configuration{
	Magic:               7630401,
	Version:             23,
	SeedList:            []string{"127.0.0.1:30338"},
	HttpInfoPort:        20333,
	HttpInfoStart:       true,
	HttpRestPort:        20334,
	HttpWsPort:          20335,
	WsHeartbeatInterval: 60,
	HttpJsonPort:        20336,
	NodePort:            20338,
	NodeOpenPort:        20866,
	OpenService:         true,
	PrintLevel:          0,
	MaxLogsSize:         0,
	MaxPerLogSize:       0,
	IsTLS:               false,
	CertPath:            "./sample-cert.pem",
	KeyPath:             "./sample-cert-key.pem",
	CAPath:              "./sample-ca.pem",
	MultiCoreNum:        4,
	MaxTxsInBlock:       10000,
	MaxBlockSize:        8000000,
	MinCrossChainTxFee:  10000,
	PowConfiguration: config.PowConfiguration{
		PayToAddr:  "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
		AutoMining: false,
		MinerInfo:  "ELA",
		MinTxFee:   100,
		ActiveNet:  "RegNet",
	},
	EnableArbiter: false,
	ArbiterConfiguration: config.ArbiterConfiguration{
		Name:             "test",
		Magic:            7630403,
		NodePort:         30338,
		ProtocolVersion:  0,
		Services:         0,
		PrintLevel:       1,
		SignTolerance:    5,
		MaxLogsSize:      0,
		MaxPerLogSize:    0,
		MaxConnections:   100,
		MajorityCount:    3,
		ArbitratorsCount: 5,
		CandidatesCount:  0,
	},
}
