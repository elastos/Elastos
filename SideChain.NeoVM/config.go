package main

import (
	"encoding/json"
	"io/ioutil"
	"bytes"
	"strings"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/common/config"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/params"

)

const (
	configFilename        = "./config.json"
	defaultLogLevel       = elalog.LevelInfo
	defaultLogDir         = "logs"
)

var (
	// spvNetParams defines the SPV module network parameters.
	spvNetParams = &config.DefaultParams

	// activeNetParams defines the side chain network parameters.
	activeNetParams = &params.MainNetParams

	// defaultConfig defines the default configuration parameters.
	defaultConfig = configParams{
		LogLevel: defaultLogLevel,
	}

	// Load configuration from file.
	cfg = loadConfig()
)

type configParams struct {
	ActiveNet          string
	Magic              uint32
	NodePort           uint16
	DNSSeeds           []string
	DisableDNS         bool
	PermanentPeers     []string
	SPVMagic           uint32
	SPVDNSSeeds        []string
	SPVDisableDNS      bool
	SPVPermanentPeers  []string
	EnableREST         bool
	RESTPort           uint16
	EnableWS           bool
	WSPort             uint16
	EnableRPC          bool
	RPCPort            uint16
	RPCUser            string
	RPCPass            string
	RPCWhiteList       []string
	LogLevel           elalog.Level
	LogsFolderSize     int64
	PerLogFileSize     int64
	FoundationAddress  string
	DisableTxFilters   bool
	ExchangeRate       float64
	MinCrossChainTxFee int64
	EnableMining       bool
	InstantBlock       bool
	PayToAddr          string
	MinerInfo          string
}

func loadConfigFile() *configParams {
	file, err := ioutil.ReadFile(configFilename)
	if err != nil {
		return &defaultConfig
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	// We have put the default configuration into config file, it's not mater
	// whether unmarshall success or not.
	json.Unmarshal(file, &defaultConfig)

	return &defaultConfig
}

// loadConfig load the configuration parameters to running the DID node.
func loadConfig() *configParams {
	cfg := loadConfigFile()
	switch strings.ToLower(cfg.ActiveNet) {
	case "testnet", "test", "t":
		testNetDefault(cfg)
		spvNetParams = config.DefaultParams.TestNet()
		activeNetParams = &params.TestNetParams

	case "regnet", "reg", "r":
		regNetDefault(cfg)
		spvNetParams = config.DefaultParams.RegNet()
		activeNetParams = &params.RegNetParams

	default:
		mainNetDefault(cfg)
		spvNetParams = &config.DefaultParams
		activeNetParams = &params.MainNetParams

	}

	if cfg.Magic > 0 {
		activeNetParams.Magic = cfg.Magic
	}
	if len(cfg.DNSSeeds) > 0 {
		activeNetParams.DNSSeeds = cfg.DNSSeeds
	}
	if cfg.DisableDNS {
		activeNetParams.DNSSeeds = nil
	}
	if cfg.NodePort > 0 {
		activeNetParams.DefaultPort = cfg.NodePort
	}
	if len(cfg.FoundationAddress) > 0 {
		foundation, err := common.Uint168FromAddress(cfg.FoundationAddress)
		if err == nil {
			spvNetParams.Foundation = *foundation
			spvNetParams.GenesisBlock = config.GenesisBlock(foundation)
			activeNetParams.Foundation = *foundation
		}
	}
	if cfg.SPVMagic > 0 {
		spvNetParams.Magic = cfg.SPVMagic
	}
	if len(cfg.SPVDNSSeeds) > 0 {
		spvNetParams.DNSSeeds = cfg.SPVDNSSeeds
	}
	if cfg.SPVDisableDNS {
		spvNetParams.DNSSeeds = nil
	}
	if cfg.ExchangeRate > 0 {
		activeNetParams.ExchangeRate = cfg.ExchangeRate
	}
	if cfg.DisableTxFilters {
		activeNetParams.DisableTxFilters = cfg.DisableTxFilters
	}
	if cfg.MinCrossChainTxFee > 0 {
		activeNetParams.MinCrossChainTxFee = cfg.MinCrossChainTxFee
	}
	if cfg.InstantBlock {
		params.InstantBlock(activeNetParams)
	}

	return cfg
}


// mainNetDefault set the default parameters for main network usage.
func mainNetDefault(cfg *configParams) {
	if cfg.RESTPort == 0 {
		cfg.RESTPort = 20624
	}
	if cfg.WSPort == 0 {
		cfg.WSPort = 20625
	}
	if cfg.RPCPort == 0 {
		cfg.RPCPort = 20626
	}
}

// testNetDefault set the default parameters for test network usage.
func testNetDefault(cfg *configParams) {
	if cfg.RESTPort == 0 {
		cfg.RESTPort = 21624
	}
	if cfg.WSPort == 0 {
		cfg.WSPort = 21625
	}
	if cfg.RPCPort == 0 {
		cfg.RPCPort = 21626
	}
}

// regNetDefault set the default parameters for regression network usage.
func regNetDefault(cfg *configParams) {
	if cfg.RESTPort == 0 {
		cfg.RESTPort = 22624
	}
	if cfg.WSPort == 0 {
		cfg.WSPort = 22625
	}
	if cfg.RPCPort == 0 {
		cfg.RPCPort = 22626
	}
}
