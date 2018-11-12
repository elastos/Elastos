package config

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"time"
)

const (
	DefaultConfigFilename = "./arbiter.conf"
)

var (
	Version    string
	Parameters configParams
)

type Configuration struct {
	Name          string `json:"Name"`
	SignTolerance uint64 `json:"SignTolerance"`
	SleepDuration uint64 `json:"SleepDuration"`
	IsProducer    bool   `json:"IsProducer"`
	IsArbitrator  bool   `json:"IsArbitrator"`

	Magic    uint32   `json:"Magic"`
	Version  int      `json:"Version"`
	SeedList []string `json:"SeedList"`
	NodePort uint16   `json:"NodePort"`

	MainNode     *MainNodeConfig   `json:"MainNode"`
	SideNodeList []*SideNodeConfig `json:"SideNodeList"`

	SyncInterval  time.Duration `json:"SyncInterval"`
	HttpJsonPort  int           `json:"HttpJsonPort"`
	HttpRestPort  uint16        `json:"HttpRestPort"`
	PrintLevel    int           `json:"PrintLevel"`
	SpvPrintLevel int           `json:"SpvPrintLevel"`
	SPVLogPath    string        `json:"SPVLogPath"`
	MaxLogSize    int64         `json:"MaxLogSize"`
	MaxPerLogSize int64         `json:"MaxPerLogSize"`

	SideChainMonitorScanInterval time.Duration `json:"SideChainMonitorScanInterval"`
	ClearTransactionInterval     time.Duration `json:"ClearTransactionInterval"`
	MinReceivedUsedUtxoMsgNumber uint32        `json:"MinReceivedUsedUtxoMsgNumber"`
	MinOutbound                  int           `json:"MinOutbound"`
	MaxConnections               int           `json:"MaxConnections"`
	SideAuxPowFee                int           `json:"SideAuxPowFee"`
	MinThreshold                 int           `json:"MinThreshold"`
	DepositAmount                int           `json:"DepositAmount"`
}

type RpcConfig struct {
	IpAddress    string `json:"IpAddress"`
	HttpJsonPort int    `json:"HttpJsonPort"`
}

type MainNodeConfig struct {
	Rpc               *RpcConfig `json:"Rpc"`
	SpvSeedList       []string   `json:"SpvSeedList""`
	Magic             uint32     `json:"Magic"`
	MinOutbound       int        `json:"MinOutbound"`
	MaxConnections    int        `json:"MaxConnections"`
	FoundationAddress string     `json:"FoundationAddress"`
}

type SideNodeConfig struct {
	Rpc *RpcConfig `json:"Rpc"`

	ExchangeRate        float64 `json:"ExchangeRate"`
	GenesisBlockAddress string  `json:"GenesisBlockAddress"`
	GenesisBlock        string  `json:"GenesisBlock"`
	KeystoreFile        string  `json:"KeystoreFile"`
	PayToAddr           string  `json:"PayToAddr"`
}

type ConfigFile struct {
	ConfigFile Configuration `json:"Configuration"`
}

type configParams struct {
	*Configuration
}

func GetRpcConfig(genesisBlockHash string) (*RpcConfig, bool) {
	for _, node := range Parameters.SideNodeList {
		if node.GenesisBlock == genesisBlockHash {
			return node.Rpc, true
		}
	}
	return nil, false
}

func Init() {
	file, e := ioutil.ReadFile(DefaultConfigFilename)
	if e != nil {
		fmt.Printf("File error: %v\n", e)
		os.Exit(1)
	}

	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	config := ConfigFile{
		ConfigFile: Configuration{
			Magic:                        0,
			Version:                      0,
			NodePort:                     20538,
			HttpJsonPort:                 20536,
			HttpRestPort:                 20534,
			PrintLevel:                   1,
			SpvPrintLevel:                1,
			MaxLogSize:                   0,
			SyncInterval:                 1000,
			SideChainMonitorScanInterval: 1000,
			ClearTransactionInterval:     60000,
			MinReceivedUsedUtxoMsgNumber: 2,
			MinOutbound:                  3,
			MaxConnections:               8,
			SideAuxPowFee:                50000,
			MinThreshold:                 10000000,
			DepositAmount:                10000000,
		},
	}
	e = json.Unmarshal(file, &config)
	if e != nil {
		fmt.Printf("Unmarshal json file erro %v", e)
		os.Exit(1)
	}

	Parameters.Configuration = &(config.ConfigFile)

	var out bytes.Buffer
	err := json.Indent(&out, file, "", "")
	if err != nil {
		fmt.Printf("Config file error: %v\n", e)
		os.Exit(1)
	}
	fmt.Println(out.String())

	if Parameters.Configuration.MainNode == nil {
		fmt.Printf("Need to set main node in config file\n")
		return
	}

	if Parameters.Configuration.SideNodeList == nil {
		fmt.Printf("Need to set side node list in config file\n")
		return
	}

	//for _, node := range Parameters.SideNodeList {
	//	genesisBytes, err := HexStringToBytes(node.GenesisBlock)
	//	if err != nil {
	//		fmt.Printf("Side node genesis block hash error: %v\n", e)
	//		return
	//	}
	//	reversedGenesisBytes := BytesReverse(genesisBytes)
	//	reversedGenesisStr := BytesToHexString(reversedGenesisBytes)
	//	genesisBlockHash, err := Uint256FromHexString(reversedGenesisStr)
	//	if err != nil {
	//		fmt.Printf("Side node genesis block hash reverse error: %v\n", e)
	//		return
	//	}
	//	address, err := common.GetGenesisAddress(*genesisBlockHash)
	//	if err != nil {
	//		fmt.Printf("Side node genesis block hash to address error: %v\n", e)
	//		return
	//	}
	//	node.GenesisBlockAddress = address
	//	node.GenesisBlock = reversedGenesisStr
	//}
}
