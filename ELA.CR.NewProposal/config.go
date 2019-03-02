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

func convertArbitrators(arbiters []config.CRCArbiterInfo) (result []config.CRCArbiter, err error) {
	for _, v := range arbiters {
		arbiterByte, err := common.HexStringToBytes(v.PublicKey)
		if err != nil {
			return nil, err
		}
		result = append(result, config.CRCArbiter{PublicKey: arbiterByte, NetAddress: v.NetAddress})
	}

	return result, nil
}
