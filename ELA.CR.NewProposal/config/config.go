package config

import (
	"bytes"
	"encoding/json"
	"errors"
	"io/ioutil"
	"log"
	"math/big"
	"os"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	DefaultConfigFilename = "./config.json"
	MINGENBLOCKTIME       = 2
	DefaultGenBlockTime   = 6
)

var (
	Parameters configParams
	Version    string
	mainNet    = &ChainParams{
		Name:               "MainNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x1f0008ff,
		TargetTimePerBlock: time.Minute * 2,
		TargetTimespan:     time.Minute * 2 * 720,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
	}
	testNet = &ChainParams{
		Name:               "TestNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x1e1da5ff,
		TargetTimePerBlock: time.Second * 10,
		TargetTimespan:     time.Second * 10 * 10,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
	}
	regNet = &ChainParams{
		Name:               "RegNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x207fffff,
		TargetTimePerBlock: time.Second * 1,
		TargetTimespan:     time.Second * 1 * 10,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
	}
)

type PowConfiguration struct {
	PayToAddr  string `json:"PayToAddr"`
	AutoMining bool   `json:"AutoMining"`
	MinerInfo  string `json:"MinerInfo"`
	MinTxFee   int    `json:"MinTxFee"`
	ActiveNet  string `json:"ActiveNet"`
}

type Configuration struct {
	Magic               uint32           `json:"Magic"`
	FoundationAddress   string           `json:"FoundationAddress"`
	Version             int              `json:"Version"`
	SeedList            []string         `json:"SeedList"`
	HttpRestPort        int              `json:"HttpRestPort"`
	MinCrossChainTxFee  int              `json:"MinCrossChainTxFee"`
	RestCertPath        string           `json:"RestCertPath"`
	RestKeyPath         string           `json:"RestKeyPath"`
	HttpInfoPort        uint16           `json:"HttpInfoPort"`
	HttpInfoStart       bool             `json:"HttpInfoStart"`
	OpenService         bool             `json:"OpenService"`
	HttpWsPort          int              `json:"HttpWsPort"`
	WsHeartbeatInterval time.Duration    `json:"WsHeartbeatInterval"`
	HttpJsonPort        int              `json:"HttpJsonPort"`
	OauthServerUrl      string           `json:"OauthServerUrl"`
	NoticeServerUrl     string           `json:"NoticeServerUrl"`
	NodePort            uint16           `json:"NodePort"`
	NodeOpenPort        uint16           `json:"NodeOpenPort"`
	PrintLevel          uint8            `json:"PrintLevel"`
	IsTLS               bool             `json:"IsTLS"`
	CertPath            string           `json:"CertPath"`
	KeyPath             string           `json:"KeyPath"`
	CAPath              string           `json:"CAPath"`
	MultiCoreNum        uint             `json:"MultiCoreNum"`
	MaxLogsSize         int64            `json:"MaxLogsSize"`
	MaxPerLogSize       int64            `json:"MaxPerLogSize"`
	MaxTxsInBlock       int              `json:"MaxTransactionInBlock"`
	MaxBlockSize        int              `json:"MaxBlockSize"`
	PowConfiguration    PowConfiguration `json:"PowConfiguration"`
	Arbiters            []string         `json:"Arbiters"`
	EnableArbiter       bool             `json:"EnableArbiter"`
}

type ConfigFile struct {
	ConfigFile Configuration `json:"Configuration"`
}

type ChainParams struct {
	Name               string
	PowLimit           *big.Int
	PowLimitBits       uint32
	TargetTimePerBlock time.Duration
	TargetTimespan     time.Duration
	AdjustmentFactor   int64
	MaxOrphanBlocks    int
	MinMemoryNodes     uint32
	CoinbaseLockTime   uint32
}

type configParams struct {
	*Configuration
	ChainParam *ChainParams
}

func init() {
	file, e := ioutil.ReadFile(DefaultConfigFilename)
	if e != nil {
		log.Fatalf("File error: %v\n", e)
		os.Exit(1)
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	config := ConfigFile{}
	e = json.Unmarshal(file, &config)
	if e != nil {
		log.Fatalf("Unmarshal json file erro %v", e)
		os.Exit(1)
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

func (config *Configuration) GetArbitrators() ([][]byte, error) {
	//todo finish this when arbitrator election scenario is done
	if len(config.Arbiters) == 0 {
		return nil, errors.New("arbiters not configured")
	}

	var arbitersByte [][]byte
	for _, arbiter := range config.Arbiters {
		arbiterByte, err := common.HexStringToBytes(arbiter)
		if err != nil {
			return nil, err
		}
		arbitersByte = append(arbitersByte, arbiterByte)
	}

	return arbitersByte, nil
}
