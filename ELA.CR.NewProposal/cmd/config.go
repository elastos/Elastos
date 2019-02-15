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

func init() {
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	//seed transaction nonce
	rand.Seed(time.Now().UnixNano())
}
