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
	activeNetParams = &config.MainNetParams

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
		activeNetParams.Foundation = *foundation
		activeNetParams.GenesisBlock = config.GenesisBlock(foundation)
	}
	if len(cfg.ArbiterConfiguration.OriginArbiters) > 0 {
		activeNetParams.OriginArbiters = cfg.ArbiterConfiguration.OriginArbiters
	}
	if len(cfg.ArbiterConfiguration.CRCArbiters) > 0 {
		arbiters, err := convertArbitrators(cfg.ArbiterConfiguration.CRCArbiters)
		if err == nil {
			activeNetParams.CRCArbiters = arbiters
		}
	}
	if len(cfg.HeightVersions) > 0 {
		activeNetParams.HeightVersions = cfg.HeightVersions
		if len(cfg.HeightVersions) > 3 {
			activeNetParams.VoteStartHeight = cfg.HeightVersions[1]
		}
	}
	if cfg.ArbiterConfiguration.InactiveDuration > 0 {
		activeNetParams.InactiveDuration =
			cfg.ArbiterConfiguration.InactiveDuration
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
	if cfg.ArbiterConfiguration.InactiveEliminateCount > 0 {
		activeNetParams.InactiveEliminateCount =
			cfg.ArbiterConfiguration.InactiveEliminateCount
	}

	return &config.Parameters
}

func convertArbitrators(arbiters []config.CRCArbitratorConfigItem) (result []config.CRCArbitratorParams, err error) {
	for _, v := range arbiters {
		arbiterByte, err := common.HexStringToBytes(v.PublicKey)
		if err != nil {
			return nil, err
		}
		result = append(result, config.CRCArbitratorParams{PublicKey: arbiterByte, NetAddress: v.NetAddress})
	}

	return result, nil
}
