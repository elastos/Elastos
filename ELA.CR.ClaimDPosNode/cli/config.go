package main

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"math/rand"
	"strings"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
)

const (
	configFilename = "./config.json"
	rootDir        = "elastos"
	dataDir        = rootDir + "/data"
)

var (
	cfg = loadConfigParams()
)

func loadConfigFile() *config.Configuration {
	file, err := ioutil.ReadFile(configFilename)
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

	case "testnet", "test":
		chainParams = config.TestNet

	case "regnet", "reg":
		chainParams = config.RegNet
	}

	config.Parameters = config.ConfigParams{
		Configuration: cfg,
		ChainParam:    &chainParams,
	}

	foundation, err := common.Uint168FromAddress(cfg.FoundationAddress)
	if err == nil {
		blockchain.FoundationAddress = *foundation
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
		PublicKey:              "023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		Magic:                  7630403,
		NodePort:               30338,
		ProtocolVersion:        0,
		Services:               0,
		PrintLevel:             1,
		SignTolerance:          5,
		MaxLogsSize:            0,
		MaxPerLogSize:          0,
		MaxConnections:         100,
		NormalArbitratorsCount: 5,
		CandidatesCount:        0,
		MajorityCount:          0,
	},
}

func init() {
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	//seed transaction nonce
	rand.Seed(time.Now().UnixNano())
}
