package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
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
func loadConfigFile(confPath string) (*config.Configuration, error) {
	file, err := ioutil.ReadFile(confPath)
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
func loadConfigParams(fileConfig *config.Configuration) (*config.Configuration, error) {
	switch strings.ToLower(fileConfig.ActiveNet) {
	case "testnet", "test":
		testNetDefault(fileConfig)
		activeNetParams = activeNetParams.TestNet()

	case "regnet", "reg":
		regNetDefault(fileConfig)
		activeNetParams = activeNetParams.RegNet()

	default:
		mainNetDefault(fileConfig)
	}

	config.Parameters = fileConfig
	if fileConfig.PowConfiguration.InstantBlock {
		activeNetParams = activeNetParams.InstantBlock()
	}
	if fileConfig.Magic > 0 {
		activeNetParams.Magic = fileConfig.Magic
	}
	if fileConfig.NodePort > 0 {
		activeNetParams.DefaultPort = fileConfig.NodePort
	}
	if len(fileConfig.SeedList) > 0 {
		activeNetParams.SeedList = fileConfig.SeedList
	}
	if fileConfig.MinCrossChainTxFee > 0 {
		activeNetParams.MinCrossChainTxFee = fileConfig.MinCrossChainTxFee
	}
	if fileConfig.FoundationAddress != "" {
		foundation, err := common.Uint168FromAddress(fileConfig.FoundationAddress)
		if err != nil {
			return nil, errors.New("invalid foundation address")
		}
		activeNetParams.Foundation = *foundation
		activeNetParams.GenesisBlock = config.GenesisBlock(foundation)
	}
	if fileConfig.CRCAddress != "" {
		crcAddress, err := common.Uint168FromAddress(fileConfig.CRCAddress)
		if err != nil {
			return nil, errors.New("invalid CRC address")
		}
		activeNetParams.CRCAddress = *crcAddress
	}
	if fileConfig.VoteStartHeight > 0 {
		activeNetParams.VoteStartHeight = fileConfig.VoteStartHeight
	}
	if fileConfig.CheckAddressHeight > 0 {
		activeNetParams.CheckAddressHeight = fileConfig.CheckAddressHeight
	}
	if fileConfig.CRCOnlyDPOSHeight > 0 {
		activeNetParams.CRCOnlyDPOSHeight = fileConfig.CRCOnlyDPOSHeight
	}
	if fileConfig.PublicDPOSHeight > 0 {
		activeNetParams.PublicDPOSHeight = fileConfig.PublicDPOSHeight
	}

	// When arbiter service enabled, IP address must be set.
	enableArbiter := fileConfig.DPoSConfiguration.EnableArbiter
	if enableArbiter {
		ipAddr := fileConfig.DPoSConfiguration.IPAddress
		if ipAddr == "" {
			return nil, errors.New("arbiter IPAddress must set when arbiter service enabled")
		}
		var err error
		fileConfig.DPoSConfiguration.IPAddress, err = hostToIP(ipAddr)
		if err != nil {
			return nil, errors.New(fmt.Sprintf("invalid IP %s, %s", ipAddr, err))
		}
	}

	// FIXME we should replace the default value in activeNetParams by the
	// loaded parameters from file, this will be fixed after the arbiter use
	// the activeNetParams instead of using the global config.
	if fileConfig.DPoSConfiguration.Magic == 0 {
		fileConfig.DPoSConfiguration.Magic = activeNetParams.DPoSMagic
	}
	if fileConfig.DPoSConfiguration.DPoSPort == 0 {
		fileConfig.DPoSConfiguration.DPoSPort = activeNetParams.DPoSDefaultPort
	}

	if len(fileConfig.DPoSConfiguration.OriginArbiters) > 0 {
		activeNetParams.OriginArbiters = fileConfig.DPoSConfiguration.OriginArbiters
	}
	if len(fileConfig.DPoSConfiguration.CRCArbiters) > 0 {
		activeNetParams.CRCArbiters = fileConfig.DPoSConfiguration.CRCArbiters
	}
	if fileConfig.DPoSConfiguration.NormalArbitratorsCount > 0 {
		activeNetParams.GeneralArbiters =
			fileConfig.DPoSConfiguration.NormalArbitratorsCount
	}
	if fileConfig.DPoSConfiguration.PreConnectOffset > 0 {
		activeNetParams.PreConnectOffset =
			fileConfig.DPoSConfiguration.PreConnectOffset
	}
	if fileConfig.DPoSConfiguration.CandidatesCount > 0 {
		activeNetParams.CandidateArbiters =
			fileConfig.DPoSConfiguration.CandidatesCount
	}
	if fileConfig.DPoSConfiguration.SignTolerance > 0 {
		activeNetParams.ToleranceDuration =
			fileConfig.DPoSConfiguration.SignTolerance * time.Second
	}
	if fileConfig.DPoSConfiguration.MaxInactiveRounds > 0 {
		activeNetParams.MaxInactiveRounds =
			fileConfig.DPoSConfiguration.MaxInactiveRounds
	}
	if fileConfig.DPoSConfiguration.InactivePenalty > 0 {
		activeNetParams.InactivePenalty =
			fileConfig.DPoSConfiguration.InactivePenalty
	}
	if fileConfig.DPoSConfiguration.EmergencyInactivePenalty > 0 {
		activeNetParams.EmergencyInactivePenalty =
			fileConfig.DPoSConfiguration.EmergencyInactivePenalty
	}

	return fileConfig, nil
}

// hostToIP parse the host to IP address.
func hostToIP(host string) (string, error) {
	// Skip if host is already an IP address.
	if ip := net.ParseIP(host); ip != nil {
		return ip.String(), nil
	}

	// Attempt to look up an IP address associated with the parsed host.
	ips, err := net.LookupIP(host)
	if err != nil {
		return "", err
	}
	if len(ips) == 0 {
		return "", fmt.Errorf("no addresses found for %s", host)
	}

	return ips[0].String(), nil
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
