package main

import (
	"encoding/json"
	"fmt"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"io/ioutil"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA.SideChain.ID/params"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
)

const (
	configFilename        = "./did.conf"
	defaultLogLevel       = elalog.LevelInfo
	defaultLogFolderSize  = 2 * elalog.GBSize  // 2 GB
	defaultMaxLogFileSize = 20 * elalog.MBSize // 20 MB
	defaultLogDir         = "./logs/"
	defaultDataDir        = "./"
)

var (
	// Set default active net params.
	activeNetParams = &params.MainNetParams

	// Load configuration from file.
	cfg, loadConfigErr = loadConfig()
)

type config struct {
	// Network settings.
	ActiveNet   string   `json:"ActiveNet"`
	Magic       uint32   `json:"Magic"`
	DefaultPort uint16   `json:"DefaultPort"`
	SeedList    []string `json:"SeedList"`

	// SPV config
	SpvConfig *spvconfig `json:"SpvConfig,omitempty"`

	// Chain parameters.
	Foundation         string  `json:"Foundation"`
	MinTxFee           int64   `json:"MinTxFee"`
	ExchangeRate       float64 `json:"ExchangeRate"`
	DisableTxFilters   bool    `json:"DisableTxFilters"`
	MinCrossChainTxFee int     `json:"MinCrossChainTxFee"`

	// Mining parameters.
	Mining    bool   `json:"Mining"`
	MinerInfo string `json:"MinerInfo"`
	MinerAddr string `json:"MinerAddr"`

	// System settings.
	LogLevel          elalog.Level `json:"LogLevel"`
	MaxLogsFolderSize int64        `json:"MaxLogsFolderSize"`
	MaxPerLogFileSize int64        `json:"MaxPerLogFileSize"`
	HttpInfoStart     bool         `json:"HttpInfoStart"`
	HttpInfoPort      uint16       `json:"HttpInfoPort"`
	HttpRestPort      uint16       `json:"HttpRestPort"`
	HttpJsonPort      uint16       `json:"HttpJsonPort"`
	PrintSyncState    bool         `json:"PrintSyncState"`

	dataDir string
}

type spvconfig struct {
	// Magic defines the magic number of the main chain peer-to-peer network.
	Magic uint32

	// The public seed peers addresses.
	SeedList []string

	// NodePort is the default port for public peers provide services.
	DefaultPort uint16

	// The foundation address of the genesis block, which is different between
	// MainNet, TestNet, RegNet etc.
	Foundation string
}

func loadConfig() (*config, error) {
	cfg := config{
		LogLevel:          defaultLogLevel,
		MaxLogsFolderSize: defaultLogFolderSize,
		MaxPerLogFileSize: defaultMaxLogFileSize,
		HttpInfoStart:     true,
		HttpInfoPort:      30603,
		HttpRestPort:      30604,
		HttpJsonPort:      30606,
		dataDir:           defaultDataDir,
	}

	_, err := os.Stat(configFilename)
	if os.IsNotExist(err) {
		return &cfg, nil
	}

	data, err := ioutil.ReadFile(configFilename)
	if err != nil {
		return &cfg, err
	}
	if err := json.Unmarshal(data, &cfg); err != nil {
		return &cfg, err
	}

	switch strings.ToLower(cfg.ActiveNet) {
	case "mainnet":
		//	nothing to do
	case "testnet":
		activeNetParams = &params.TestNetParams
		cfg.dataDir = "./data_test"

	case "regnet":
		activeNetParams = &params.RegNetParams
		cfg.dataDir = "./data_regt"

	default:
		return &cfg, fmt.Errorf("unknown active net type")
	}

	if cfg.Magic > 0 {
		activeNetParams.Magic = cfg.Magic
	}
	if cfg.DefaultPort > 0 {
		activeNetParams.DefaultPort = cfg.DefaultPort
	}
	if len(cfg.SeedList) > 0 {
		activeNetParams.SeedList = cfg.SeedList
	}

	if len(cfg.Foundation) > 0 {
		foundation, err := common.Uint168FromAddress(cfg.Foundation)
		if err == nil {
			activeNetParams.Foundation = *foundation
		}
	}
	if cfg.MinTxFee > 0 {
		activeNetParams.MinTransactionFee = cfg.MinTxFee
	}
	if cfg.ExchangeRate > 0 {
		activeNetParams.ExchangeRate = cfg.ExchangeRate
	}
	if cfg.DisableTxFilters {
		activeNetParams.DisableTxFilters = true
	}
	if cfg.MinCrossChainTxFee > 0 {
		activeNetParams.MinCrossChainTxFee = cfg.MinCrossChainTxFee
	}

	if cfg.SpvConfig == nil {
		return &cfg, nil
	}

	if cfg.SpvConfig.Magic > 0 {
		activeNetParams.SpvParams.Magic = cfg.SpvConfig.Magic
	}
	if cfg.SpvConfig.DefaultPort > 0 {
		activeNetParams.SpvParams.DefaultPort = cfg.SpvConfig.DefaultPort
	}
	if len(cfg.SpvConfig.SeedList) > 0 {
		activeNetParams.SpvParams.SeedList = cfg.SpvConfig.SeedList
	}
	if len(cfg.SpvConfig.Foundation) > 0 {
		activeNetParams.SpvParams.Foundation = cfg.SpvConfig.Foundation
	}

	return &cfg, nil
}
