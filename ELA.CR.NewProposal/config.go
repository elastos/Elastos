package main

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"strings"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
)

const (
	configFilename = "./config.json"
	rootDir        = "elastos"
	dataDir        = rootDir + "/data"
)

var (
	activeNetParams = &config.DefaultParams

	cfg = loadConfigParams()
)

func loadConfigFile() *config.Configuration {
	file, err := ioutil.ReadFile(configFilename)
	if err != nil {
		return &config.Template
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	var cfgFile config.ConfigFile
	if err := json.Unmarshal(file, &cfgFile); err != nil {
		return &config.Template
	}

	return &cfgFile.Configuration
}

func loadConfigParams() *config.ConfigParams {
	cfg := loadConfigFile()

	var chainParams = config.MainNet
	switch strings.ToLower(cfg.ActiveNet) {
	case "testnet", "test":
		chainParams = config.TestNet
		activeNetParams = activeNetParams.TestNet()

	case "regnet", "reg":
		chainParams = config.RegNet
		activeNetParams = activeNetParams.RegNet()
	}

	config.Parameters = config.ConfigParams{
		Configuration: cfg,
		ChainParam:    &chainParams,
	}
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
	if len(cfg.ArbiterConfiguration.OriginArbiters) > 0 {
		activeNetParams.OriginArbiters = cfg.ArbiterConfiguration.OriginArbiters
	}
	if len(cfg.ArbiterConfiguration.CRCArbiters) > 0 {
		activeNetParams.CRCArbiters = cfg.ArbiterConfiguration.CRCArbiters
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
	if cfg.ArbiterConfiguration.NormalArbitratorsCount > 0 {
		activeNetParams.GeneralArbiters =
			cfg.ArbiterConfiguration.NormalArbitratorsCount
	}
	if cfg.ArbiterConfiguration.PreConnectOffset > 0 {
		activeNetParams.PreConnectOffset =
			cfg.ArbiterConfiguration.PreConnectOffset
	}
	if cfg.ArbiterConfiguration.CandidatesCount > 0 {
		activeNetParams.CandidateArbiters =
			cfg.ArbiterConfiguration.CandidatesCount
	}
	if cfg.ArbiterConfiguration.MaxInactiveRounds > 0 {
		activeNetParams.MaxInactiveRounds =
			cfg.ArbiterConfiguration.MaxInactiveRounds
	}
	if cfg.ArbiterConfiguration.InactivePenalty > 0 {
		activeNetParams.InactivePenalty =
			cfg.ArbiterConfiguration.InactivePenalty
	}
	if cfg.ArbiterConfiguration.EmergencyInactivePenalty > 0 {
		activeNetParams.EmergencyInactivePenalty =
			cfg.ArbiterConfiguration.EmergencyInactivePenalty
	}

	return &config.Parameters
}
