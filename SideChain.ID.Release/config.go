package main

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.ID/params"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/go-ini/ini"
)

const (
	configFilename        = "./did.conf"
	defaultLogLevel       = "info"
	defaultLogsFolderSize = 2 * elalog.GBSize  // 2 GB
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

type appconfig struct {
	TestNet            bool     `ini:"testnet" comment:"Use the test network"`
	RegTest            bool     `ini:"regtest" comment:"Use the regression test network"`
	Magic              uint32   `ini:"magic" comment:"Magic number of the peer-to-peer network"`
	DefaultPort        uint16   `ini:"port" comment:"The default port for the peer-to-peer network"`
	SeedPeers          []string `ini:"seedpeer,omitempty,allowshadow" comment:"Add a seed peer to connect with at startup"`
	Listeners          []string `ini:"listen,omitempty,allowshadow" comment:"Add an interface/port to listen for connections (default all interfaces port: 20608, testnet: 21608)"`
	DisableTxFilters   bool     `ini:"notxfilter" comment:"Disable transaction filtering support"`
	Foundation         string   `ini:"foundation" comment:"The specified payment of foundation address to use for receive mine and fee rewards"`
	MinTxFee           int64    `ini:"mintxfee" comment:"The minimum transaction fee in ELA/kB to be considered a non-zero fee"`
	MinCrossChainTxFee int64    `ini:"mincrosschaintxfee" comment:"The minimum cross chain transaction fee in ELA/kB to be considered a non-zero fee"`
	ExchangeRate       float64  `ini:"exchangeragte" comment:"The exchange rate between side chain and main chain in ELA(side)/ELA(main)"`
	HttpRestPort       uint16   `ini:"restport" comment:"Specify the interface/port to listen on to provide RESTful service (default interfaces port: 30604)"`
	HttpJsonPort       uint16   `ini:"jsonport" comment:"Specify the interface/port to listen on to provide JSON-RPC service (default interfaces port: 30606)"`
	Mining             bool     `ini:"mine" comment:"Generate (mine) ELA coins using the CPU"`
	MinerInfo          string   `ini:"minerinfo" comment:"The miner information to be used when creating a block"`
	MinerAddr          string   `ini:"mineraddr" comment:"The specified payment address to use for mining blocks"`
	LogLevel           string   `ini:"loglevel" comment:"Logging level for all subsystems {debug, info, warn, error, fatal}"`
	MaxLogsFolderSize  int64    `ini:"logsfolderlimit" comment:"Maximum total size(MB) of log files can be reserved in logs folder (default 2GB)"`
	MaxPerLogFileSize  int64    `ini:"logfilelimit" comment:"Maximum size of each log file in MB (default 20MB)"`
	MonitorState       bool     `ini:"monitorstate" comment:"Print logs in 10 second interval to monitor node connections and sync status"`
	dataDir            string
}

type spvconfig struct {
	TestNet     bool     `ini:"testnet" comment:"Use the test network"`
	RegTest     bool     `ini:"regtest" comment:"Use the regression test network"`
	Magic       uint32   `ini:"magic" comment:"Magic number of the peer-to-peer network"`
	SeedPeers   []string `ini:"seedpeer,omitempty,allowshadow" comment:"Add a seed peer to connect with at startup"`
	DefaultPort uint16   `ini:"port" comment:"The port to connect with when connect to a peer"`
	Foundation  string   `ini:"foundation" comment:"The specified payment of foundation address to use for receive mine and fee rewards"`
}

func loadConfig() (*appconfig, error) {
	appCfg := appconfig{
		LogLevel:          defaultLogLevel,
		MaxLogsFolderSize: defaultLogsFolderSize,
		MaxPerLogFileSize: defaultMaxLogFileSize,
		HttpRestPort:      30604,
		HttpJsonPort:      30606,
		dataDir:           defaultDataDir,
	}
	spvCfg := spvconfig{}

	if !loadOldConfig(&appCfg, &spvCfg) {
		// Load configuration from file.
		file, err := ini.ShadowLoad(configFilename)
		if err != nil {
			return &appCfg, nil
		}

		// Map basic options in Application section
		err = file.Section("Application").MapTo(&appCfg)
		if err != nil {
			return &appCfg, err
		}
		// Map SPV options in SPV Options section
		err = file.Section("SPV Options").MapTo(&spvCfg)
		if err != nil {
			return &appCfg, err
		}
	}

	// Multiple networks can't be selected simultaneously.
	numNets := 0
	// Count number of network flags passed; assign active network params
	// while we're at it
	if appCfg.TestNet {
		numNets++
		appCfg.dataDir = "./data_test"
		activeNetParams = &params.TestNetParams
	}
	if appCfg.RegTest {
		numNets++
		appCfg.dataDir = "./data_regt"
		activeNetParams = &params.RegNetParams
	}
	if numNets > 1 {
		return &appCfg, errors.New("[Application] The testnet and regtest" +
			" params can't be used together -- choose one of the two")
	}

	if appCfg.Magic > 0 {
		activeNetParams.Magic = appCfg.Magic
	}
	if len(appCfg.SeedPeers) > 0 {
		activeNetParams.SeedList = appCfg.SeedPeers
	}
	if appCfg.DefaultPort > 0 {
		activeNetParams.DefaultPort = appCfg.DefaultPort
	}

	if len(appCfg.Foundation) > 0 {
		foundation, err := common.Uint168FromAddress(appCfg.Foundation)
		if err == nil {
			activeNetParams.Foundation = *foundation
		}
	}
	if appCfg.MinTxFee > 0 {
		activeNetParams.MinTransactionFee = appCfg.MinTxFee
	}
	if appCfg.ExchangeRate > 0 {
		activeNetParams.ExchangeRate = appCfg.ExchangeRate
	}
	if appCfg.DisableTxFilters {
		activeNetParams.DisableTxFilters = true
	}
	if appCfg.MinCrossChainTxFee > 0 {
		activeNetParams.MinCrossChainTxFee = appCfg.MinCrossChainTxFee
	}

	// Multiple networks can't be selected simultaneously.
	numNets = 0
	// Count number of network flags passed; assign active network params
	// while we're at it
	if appCfg.TestNet {
		numNets++
		activeNetParams.SpvParams = params.TestNetSpvParams
	}
	if appCfg.RegTest {
		numNets++
		activeNetParams.SpvParams = params.RegNetSpvParams
	}
	if numNets > 1 {
		return &appCfg, errors.New("[SPV Options] The testnet and regtest" +
			" params can't be used together -- choose one of the two")
	}

	if spvCfg.Magic > 0 {
		activeNetParams.SpvParams.Magic = spvCfg.Magic
	}
	if spvCfg.DefaultPort > 0 {
		activeNetParams.SpvParams.DefaultPort = spvCfg.DefaultPort
	}
	if len(spvCfg.SeedPeers) > 0 {
		activeNetParams.SpvParams.SeedList = spvCfg.SeedPeers
	}
	if len(spvCfg.Foundation) > 0 {
		activeNetParams.SpvParams.Foundation = spvCfg.Foundation
	}

	return &appCfg, nil
}
