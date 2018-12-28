package config

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"log"
	"math/big"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	DefaultConfigFilename = "./config.json"
	MINGENBLOCKTIME       = 2
	DefaultGenBlockTime   = 6

	DataPath   = "elastos"
	DataDir    = "data"
	ChainDir   = "chain"
	DposDir    = "dpos"
	LogDir     = "logs"
	NodeDir    = "node"
	ArbiterDir = "arbiter"
)

var (
	Parameters configParams
	Version    string
)

type PowConfiguration struct {
	PayToAddr  string `json:"PayToAddr"`
	AutoMining bool   `json:"AutoMining"`
	MinerInfo  string `json:"MinerInfo"`
	MinTxFee   int    `json:"MinTxFee"`
	ActiveNet  string `json:"ActiveNet"`
}

type RpcConfiguration struct {
	User    	 string               `json:"User"`
	Pass    	 string               `json:"Pass"`
	WhiteIpList  []string             `json:"WhiteIpList"`
}

type Configuration struct {
	Magic                uint32               `json:"Magic"`
	FoundationAddress    string               `json:"FoundationAddress"`
	Version              int                  `json:"Version"`
	SeedList             []string             `json:"SeedList"`
	HttpRestPort         int                  `json:"HttpRestPort"`
	MinCrossChainTxFee   int                  `json:"MinCrossChainTxFee"`
	RestCertPath         string               `json:"RestCertPath"`
	RestKeyPath          string               `json:"RestKeyPath"`
	HttpInfoPort         uint16               `json:"HttpInfoPort"`
	HttpInfoStart        bool                 `json:"HttpInfoStart"`
	OpenService          bool                 `json:"OpenService"`
	HttpWsPort           int                  `json:"HttpWsPort"`
	WsHeartbeatInterval  time.Duration        `json:"WsHeartbeatInterval"`
	HttpJsonPort         int                  `json:"HttpJsonPort"`
	OauthServerUrl       string               `json:"OauthServerUrl"`
	NoticeServerUrl      string               `json:"NoticeServerUrl"`
	NodePort             uint16               `json:"NodePort"`
	NodeOpenPort         uint16               `json:"NodeOpenPort"`
	PrintLevel           uint8                `json:"PrintLevel"`
	IsTLS                bool                 `json:"IsTLS"`
	CertPath             string               `json:"CertPath"`
	KeyPath              string               `json:"KeyPath"`
	CAPath               string               `json:"CAPath"`
	MultiCoreNum         uint                 `json:"MultiCoreNum"`
	MaxLogsSize          int64                `json:"MaxLogsSize"`
	MaxPerLogSize        int64                `json:"MaxPerLogSize"`
	MaxTxsInBlock        int                  `json:"MaxTransactionInBlock"`
	MaxBlockSize         int                  `json:"MaxBlockSize"`
	PowConfiguration     PowConfiguration     `json:"PowConfiguration"`
	Arbiters             []string             `json:"Arbiters"`
	EnableArbiter        bool                 `json:"EnableArbiter"`
	ArbiterConfiguration ArbiterConfiguration `json:"ArbiterConfiguration"`
	RpcConfiguration     RpcConfiguration     `json:"RpcConfiguration"`
}

type ArbiterConfiguration struct {
	Name             string `json:"Name"`
	Magic            uint32 `json:"Magic"`
	NodePort         uint16 `json:"NodePort"`
	ProtocolVersion  uint32 `json:"ProtocolVersion"`
	Services         uint64 `json:"Services"`
	PrintLevel       uint8  `json:"PrintLevel"`
	SignTolerance    uint64 `json:"SignTolerance"`
	MaxLogsSize      int64  `json:"MaxLogsSize"`
	MaxPerLogSize    int64  `json:"MaxPerLogSize"`
	MaxConnections   int    `json:"MaxConnections"`
	MajorityCount    uint32 `json:"MajorityCount"`
	ArbitratorsCount uint32 `json:"ArbitratorsCount"`
	CandidatesCount  uint32 `json:"CandidatesCount"`
}

type Seed struct {
	PublicKey string `json:"PublicKey"`
	Address   string `json:"Address"`
}

type ConfigFile struct {
	ConfigFile Configuration `json:"Configuration"`
}

type ChainParams struct {
	Name               string        `json:"Name"`
	PowLimit           *big.Int      `json:"PowLimit"`
	PowLimitBits       uint32        `json:"PowLimitBits"`
	TargetTimePerBlock time.Duration `json:"TargetTimePerBlock"`
	TargetTimespan     time.Duration `json:"TargetTimespan"`
	AdjustmentFactor   int64         `json:"AdjustmentFactor"`
	MaxOrphanBlocks    int           `json:"MaxOrphanBlocks"`
	MinMemoryNodes     uint32        `json:"MinMemoryNodes"`
	CoinbaseLockTime   uint32        `json:"CoinbaseLockTime"`
}

type configParams struct {
	*Configuration
	ChainParam *ChainParams
}

func init() {
	var config ConfigFile

	if _, err := os.Stat(DefaultConfigFilename); os.IsNotExist(err) {
		config.ConfigFile = configTemplate

	} else {
		file, e := ioutil.ReadFile(DefaultConfigFilename)
		if e != nil {
			log.Fatalf("File error: %v\n", e)
			os.Exit(1)
		}
		// Remove the UTF-8 Byte Order Mark
		file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

		if e := json.Unmarshal(file, &config); e != nil {
			log.Fatalf("Unmarshal json file erro %v", e)
			os.Exit(1)
		}
	}

	//	Parameters = &(config.ConfigFile)
	Parameters.Configuration = &config.ConfigFile
	if Parameters.PowConfiguration.ActiveNet == "MainNet" {
		Parameters.ChainParam = mainNet
	} else if Parameters.PowConfiguration.ActiveNet == "TestNet" {
		Parameters.ChainParam = testNet
	} else if Parameters.PowConfiguration.ActiveNet == "RegNet" {
		Parameters.ChainParam = regNet
	}
}

func (config *Configuration) GetArbiterID() []byte {
	publicKey, err := common.HexStringToBytes(config.ArbiterConfiguration.Name)
	if err != nil || len(publicKey) != 33 {
		log.Fatalf("get arbiter public key error %v", err)
		os.Exit(1)
	}

	return publicKey
}
