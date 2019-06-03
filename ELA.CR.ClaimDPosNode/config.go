package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"io/ioutil"
	"net"
	"strings"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/utils/elalog"
)

const (
	// dataPath indicates the path storing the chain data.
	dataPath = "data"

	// logPath indicates the path storing the node log.
	nodeLogPath = "logs/node"
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
	cfg *config.Configuration
)

// loadConfigFile read configuration parameters through the config file.
func loadConfigFile(path string) (*config.Configuration, error) {
	file, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	cfgFile := struct {
		config.Configuration `json:"Configuration"`
	}{
		Configuration: defaultConfig,
	}

	err = json.Unmarshal(file, &cfgFile)
	if err != nil {
		return nil, errors.New("config file parsing failed, " + err.Error())
	}

	return &cfgFile.Configuration, nil
}

// loadConfigParams load the configuration parameters to running the ELA node.
func loadConfigParams(cfg *config.Configuration) (*config.Configuration, error) {
	switch strings.ToLower(cfg.ActiveNet) {
	case "testnet", "test":
		testNetDefault(cfg)
		activeNetParams = activeNetParams.TestNet()

	case "regnet", "reg":
		regNetDefault(cfg)
		activeNetParams = activeNetParams.RegNet()
		pact.MaxBlockSize = 2000000

	default:
		mainNetDefault(cfg)
		pact.MaxBlockSize = 2000000
	}

	if cfg.MaxBlockSize > 0 {
		pact.MaxBlockSize = cfg.MaxBlockSize
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
	if len(cfg.DNSSeeds) > 0 {
		activeNetParams.DNSSeeds = cfg.DNSSeeds
	}
	if cfg.DisableDNS {
		activeNetParams.DNSSeeds = nil
	}
	if cfg.MinCrossChainTxFee > 0 {
		activeNetParams.MinCrossChainTxFee = cfg.MinCrossChainTxFee
	}
	if cfg.FoundationAddress != "" {
		foundation, err := common.Uint168FromAddress(cfg.FoundationAddress)
		if err != nil {
			return nil, errors.New("invalid foundation address")
		}
		activeNetParams.Foundation = *foundation
		activeNetParams.GenesisBlock = config.GenesisBlock(foundation)
	}
	if cfg.CRCAddress != "" {
		crcAddress, err := common.Uint168FromAddress(cfg.CRCAddress)
		if err != nil {
			return nil, errors.New("invalid CRC address")
		}
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
	if cfg.DPoSConfiguration.EnableArbiter {
		if err := checkHost(cfg.DPoSConfiguration.IPAddress); err != nil {
			return nil, err
		}
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
	if cfg.DPoSConfiguration.SignTolerance > 0 {
		activeNetParams.ToleranceDuration =
			cfg.DPoSConfiguration.SignTolerance * time.Second
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

	return cfg, nil
}

// checkHost check the host or IP address is valid and available.
func checkHost(host string) error {
	// Empty host check.
	if host == "" {
		return errors.New("arbiter IPAddress must set when arbiter" +
			" service enabled")
	}

	// Skip if host is already an IP address.
	if ip := net.ParseIP(host); ip != nil {
		return nil
	}

	// Attempt to look up an IP address associated with the parsed host.
	ips, err := net.LookupIP(host)
	if err != nil {
		return err
	}
	if len(ips) == 0 {
		return fmt.Errorf("no addresses found for %s", host)
	}

	return nil
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
