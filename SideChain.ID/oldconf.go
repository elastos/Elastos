package main

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
)

const (
	oldConfigFilename = "./config.json"
)

// This is the deprecated old configuration parameters load from ./config.json
// file. Please use the new ini standard new did.conf instead. If both new and
// old config file are exist, old config will be ignored.
type oldconf struct {
	Configuration struct {
		Magic                      uint32
		SpvMagic                   uint32
		SeedList                   []string
		SpvSeedList                []string
		ExchangeRate               float64
		MinCrossChainTxFee         int64
		HttpRestPort               uint16
		HttpJsonPort               uint16
		NodePort                   uint16
		PrintLevel                 elalog.Level
		MaxLogsSize                int64
		MaxPerLogSize              int64
		FoundationAddress          string
		MainChainFoundationAddress string
		PowConfiguration           struct {
			PayToAddr  string
			AutoMining bool
			MinerInfo  string
			MinTxFee   int64
			ActiveNet  string
		}
	}
}

// loadOldConfig fills the parameters into new config options from the old
// config file.  Returns if load from old config file.
func loadOldConfig(appCfg *appconfig, spvCfg *spvconfig) bool {
	// If new config file exist, ignore old config file.
	_, err := os.Stat(configFilename)
	if err == nil {
		return false
	}

	data, err := ioutil.ReadFile(oldConfigFilename)
	if err != nil {
		return false
	}

	// Map Application Options.
	oldCfg := new(oldconf)
	err = json.Unmarshal(data, oldCfg)
	if err != nil {
		return false
	}

	config := oldCfg.Configuration
	powCfg := oldCfg.Configuration.PowConfiguration

	switch strings.ToLower(powCfg.ActiveNet) {
	case "testnet":
		appCfg.TestNet = true
	case "regnet":
		appCfg.RegTest = true
	}

	appCfg.Magic = config.Magic
	appCfg.SeedPeers = config.SeedList
	appCfg.DefaultPort = config.NodePort
	appCfg.DisableTxFilters = false
	appCfg.Foundation = config.FoundationAddress
	appCfg.MinTxFee = powCfg.MinTxFee
	appCfg.MinCrossChainTxFee = config.MinCrossChainTxFee
	appCfg.ExchangeRate = config.ExchangeRate
	appCfg.HttpRestPort = config.HttpRestPort
	appCfg.HttpJsonPort = config.HttpJsonPort
	appCfg.Mining = powCfg.AutoMining
	appCfg.MinerInfo = powCfg.MinerInfo
	appCfg.MinerAddr = powCfg.PayToAddr

	switch config.PrintLevel {
	case elalog.LevelDebug:
		appCfg.LogLevel = "debug"
	case elalog.LevelInfo:
		appCfg.LogLevel = "info"
	case elalog.LevelWarn:
		appCfg.LogLevel = "warn"
	case elalog.LevelError:
		appCfg.LogLevel = "error"
	case elalog.LevelFatal:
		appCfg.LogLevel = "fatal"
	case elalog.LevelOff:
		appCfg.LogLevel = "off"
	default:
		appCfg.LogLevel = "info"
	}
	appCfg.MaxLogsFolderSize = config.MaxLogsSize
	appCfg.MaxPerLogFileSize = config.MaxPerLogSize
	appCfg.MonitorState = true

	// Map SPV Options.
	spvCfg.Magic = config.SpvMagic
	spvCfg.SeedPeers = config.SpvSeedList
	spvCfg.Foundation = config.MainChainFoundationAddress

	return true
}
