package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"strings"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/utils/elalog"
)

const (
	// rootDir defines the root folder storing the ELA data.
	rootDir = "elastos"

	// dataDir defines the folder where to put the database files.
	dataDir = rootDir + "/data"

	// configFilename defines the configuration file name for the ELA node.
	configFilename = "./config.json"
)

var (
	// activeNetParams is a pointer to the parameters specific to the currently
	// active ELA network.
	activeNetParams = &config.DefaultParams

	// defaultConfig defines the default parameters to running a ELA node.
	defaultConfig = config.Configuration{
		PrintLevel: elalog.LevelInfo,
	}

	// cfg is the loaded config parameters to running the ELA node.
	cfg = loadConfigParams()
)

// loadConfigFile read configuration parameters through the config.json file.
func loadConfigFile() *config.Configuration {
	file, err := ioutil.ReadFile(configFilename)
	if err != nil {
		return &defaultConfig
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	cfgFile := struct {
		config.Configuration `json:"Configuration"`
	}{
		Configuration: defaultConfig,
	}

	// We have put the default configuration into config file, it's not mater
	// whether unmarshall success or not.
	json.Unmarshal(file, &cfgFile)

	return &cfgFile.Configuration
}

// loadConfigParams load the configuration parameters to running the ELA node.
func loadConfigParams() *config.Configuration {
	cfg := loadConfigFile()

	switch strings.ToLower(cfg.ActiveNet) {
	case "testnet", "test":
		testNetDefault(cfg)
		activeNetParams = activeNetParams.TestNet()

	case "regnet", "reg":
		regNetDefault(cfg)
		activeNetParams = activeNetParams.RegNet()

	default:
		mainNetDefault(cfg)
	}

	config.Parameters = cfg
	if cfg.PowConfiguration.InstantBlock {
		activeNetParams = activeNetParams.InstantBlock()
	}
	if cfg.Magic > 0 {
		activeNetParams.Magic = cfg.Magic
	}
	if cfg.NodePort > 0 {
		activeNetParams.DefaultPort = cfg.NodePort
	}
	if len(cfg.SeedList) > 0 {
		activeNetParams.SeedList = cfg.SeedList
	}
	if cfg.MinCrossChainTxFee > 0 {
		activeNetParams.MinCrossChainTxFee = cfg.MinCrossChainTxFee
	}
	foundation, err := common.Uint168FromAddress(cfg.FoundationAddress)
	if err == nil {
		activeNetParams.Foundation = *foundation
		activeNetParams.GenesisBlock = config.GenesisBlock(foundation)
	}
	crcAddress, err := common.Uint168FromAddress(cfg.CRCAddress)
	if err == nil {
		activeNetParams.CRCAddress = *crcAddress
	}
	if cfg.VoteStartHeight > 0 {
		activeNetParams.VoteStartHeight = cfg.VoteStartHeight
	}
	if cfg.CheckAddressHeight > 0 {
		activeNetParams.CheckAddressHeight = cfg.CheckAddressHeight
	}
	if cfg.CRCOnlyDPOSHeight > 0 {
		activeNetParams.CRCOnlyDPOSHeight = cfg.CRCOnlyDPOSHeight
	}
	if cfg.PublicDPOSHeight > 0 {
		activeNetParams.PublicDPOSHeight = cfg.PublicDPOSHeight
	}

	// When arbiter service enabled, IP address must be set.
	enableArbiter := cfg.DPoSConfiguration.EnableArbiter
	ipAddr := cfg.DPoSConfiguration.IPAddress
	if enableArbiter && ipAddr == "" {
		panic("arbiter IPAddress must set")
	}
	if ip := net.ParseIP(ipAddr); enableArbiter && ip == nil {
		panic(fmt.Sprintf("%s not a valid IP address", ipAddr))
	}

	// FIXME we should replace the default value in activeNetParams by the
	// loaded parameters from file, this will be fixed after the arbiter use
	// the activeNetParams instead of using the global config.
	if cfg.DPoSConfiguration.Magic == 0 {
		cfg.DPoSConfiguration.Magic = activeNetParams.DPoSMagic
	}
	if cfg.DPoSConfiguration.DPoSPort == 0 {
		cfg.DPoSConfiguration.DPoSPort = activeNetParams.DPoSDefaultPort
	}

	if len(cfg.DPoSConfiguration.OriginArbiters) > 0 {
		activeNetParams.OriginArbiters = cfg.DPoSConfiguration.OriginArbiters
	}
	if len(cfg.DPoSConfiguration.CRCArbiters) > 0 {
		activeNetParams.CRCArbiters = cfg.DPoSConfiguration.CRCArbiters
	}
	if cfg.DPoSConfiguration.NormalArbitratorsCount > 0 {
		activeNetParams.GeneralArbiters =
			cfg.DPoSConfiguration.NormalArbitratorsCount
	}
	if cfg.DPoSConfiguration.PreConnectOffset > 0 {
		activeNetParams.PreConnectOffset =
			cfg.DPoSConfiguration.PreConnectOffset
	}
	if cfg.DPoSConfiguration.CandidatesCount > 0 {
		activeNetParams.CandidateArbiters =
			cfg.DPoSConfiguration.CandidatesCount
	}
	if cfg.DPoSConfiguration.MaxInactiveRounds > 0 {
		activeNetParams.MaxInactiveRounds =
			cfg.DPoSConfiguration.MaxInactiveRounds
	}
	if cfg.DPoSConfiguration.InactivePenalty > 0 {
		activeNetParams.InactivePenalty =
			cfg.DPoSConfiguration.InactivePenalty
	}
	if cfg.DPoSConfiguration.EmergencyInactivePenalty > 0 {
		activeNetParams.EmergencyInactivePenalty =
			cfg.DPoSConfiguration.EmergencyInactivePenalty
	}

	return cfg
}

// mainNetDefault set the default parameters for main net usage.
func mainNetDefault(cfg *config.Configuration) {
	if cfg.HttpInfoPort == 0 {
		cfg.HttpInfoPort = 20333
	}
	if cfg.HttpRestPort == 0 {
		cfg.HttpRestPort = 20334
	}
	if cfg.HttpWsPort == 0 {
		cfg.HttpWsPort = 20335
	}
	if cfg.HttpJsonPort == 0 {
		cfg.HttpJsonPort = 20336
	}
}

// testNetDefault set the default parameters for test net usage.
func testNetDefault(cfg *config.Configuration) {
	if cfg.HttpInfoPort == 0 {
		cfg.HttpInfoPort = 21333
	}
	if cfg.HttpRestPort == 0 {
		cfg.HttpRestPort = 21334
	}
	if cfg.HttpWsPort == 0 {
		cfg.HttpWsPort = 21335
	}
	if cfg.HttpJsonPort == 0 {
		cfg.HttpJsonPort = 21336
	}
}

// regNetDefault set the default parameters for reg net usage.
func regNetDefault(cfg *config.Configuration) {
	if cfg.HttpInfoPort == 0 {
		cfg.HttpInfoPort = 22333
	}
	if cfg.HttpRestPort == 0 {
		cfg.HttpRestPort = 22334
	}
	if cfg.HttpWsPort == 0 {
		cfg.HttpWsPort = 22335
	}
	if cfg.HttpJsonPort == 0 {
		cfg.HttpJsonPort = 22336
	}
}
