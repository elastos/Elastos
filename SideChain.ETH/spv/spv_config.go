package spv

import (
	"bytes"
	"encoding/json"
	spv "github.com/elastos/Elastos.ELA.SPV/interface"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/log"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"io/ioutil"
	"math/big"
	"reflect"
	"time"
)

const (
	DefaultConfigFilename = "./spvconfig.json"
	Foundation            = "Foundation"
	CRCAddress            = "CRCAddress"
	GenesisBlock          = "GenesisBlock"
	PowLimit              = "PowLimit"
)

var PreferConfig PreferParams

type Configuration struct {
	// Exegesis in file Elastos.ELA/common/config/params.go
	Magic                    uint32         `json:"Magic"`
	DefaultPort              uint16         `json:"DefaultPort"`
	DNSSeeds                 []string       `json:"DNSSeeds"`
	ListenAddrs              []string       `json:"ListenAddrs"`
	Foundation               string         `json:"Foundation"`
	CRCAddress               string         `json:"CRCAddress"`
	PowLimit                 *big.Int       `json:"PowLimit"`
	PowLimitBits             uint32         `json:"PowLimitBits"`
	TargetTimespan           time.Duration  `json:"TargetTimespan"`
	TargetTimePerBlock       time.Duration  `json:"TargetTimePerBlock"`
	AdjustmentFactor         int64          `json:"AdjustmentFactor"`
	RewardPerBlock           common.Fixed64 `json:"RewardPerBlock"`
	CoinbaseMaturity         uint32         `json:"CoinbaseMaturity"`
	DisableTxFilters         bool           `json:"DisableTxFilters"`
	MinTransactionFee        common.Fixed64 `json:"MinTransactionFee"`
	MinCrossChainTxFee       common.Fixed64 `json:"MinCrossChainTxFee"`
	OriginArbiters           []string       `json:"OriginArbiters"`
	CheckAddressHeight       uint32         `json:"CheckAddressHeight"`
	VoteStartHeight          uint32         `json:"VoteStartHeight"`
	CRCOnlyDPOSHeight        uint32         `json:"CRCOnlyDPOSHeight"`
	PublicDPOSHeight         uint32         `json:"PublicDPOSHeight"`
	CRCArbiters              []string       `json:"CRCArbiters"`
	DPoSMagic                uint32         `json:"DPoSMagic"`
	DPoSDefaultPort          uint16         `json:"DPoSDefaultPort"`
	PreConnectOffset         uint32         `json:"PreConnectOffset"`
	GeneralArbiters          int            `json:"GeneralArbiters"`
	CandidateArbiters        int            `json:"CandidateArbiters"`
	ToleranceDuration        time.Duration  `json:"ToleranceDuration"`
	MaxInactiveRounds        uint32         `json:"MaxInactiveRounds"`
	InactivePenalty          common.Fixed64 `json:"InactivePenalty"`
	EmergencyInactivePenalty common.Fixed64 `json:"EmergencyInactivePenalty"`
	MaxLogsSize              int64          `json:"MaxLogsSize"`
	MaxPerLogSize            int64          `json:"MaxPerLogSize"`
	SpvPrintLevel            uint32         `json:"SpvPrintLevel"`
	PermanentPeers           []string       `json:"PermanentPeers"`
}

type PreferParams struct {
	Config Configuration `json:"Configuration"`
}

func init() {
	PreferConfig = PreferParams{Config: Configuration{MaxLogsSize: 0, MaxPerLogSize: 0, SpvPrintLevel: 1}}
	file, err := ioutil.ReadFile(DefaultConfigFilename)
	if err != nil {
		log.Warn("Read Spv_config file  error", "error", err)
		return
	}
	// Remove the UTF-8 Byte Ord er Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))
	err = json.Unmarshal(file, &PreferConfig)
	if err != nil {
		log.Warn("Unmarshal Spv_config file json error", "error", err)
	}
}

func ResetConfigWithReflect(params *config.Params, spvConfig *spv.Config) {
	paramsType := reflect.TypeOf(*params)
	paramsValue := reflect.ValueOf(params).Elem()
	configType := reflect.TypeOf(PreferConfig.Config)
	configValue := reflect.ValueOf(PreferConfig.Config)
	spvType := reflect.TypeOf(*spvConfig)
	spvValue := reflect.ValueOf(spvConfig).Elem()
	var destField reflect.Value
	for i := 0; i < configType.NumField(); i++ {
		name := configType.Field(i).Name
		value := configValue.Field(i)
		field := configType.Field(i)
		if _, ok := paramsType.FieldByName(name); ok {
			destField = paramsValue.FieldByName(name)
		} else if _, ok := spvType.FieldByName(name); ok {
			destField = spvValue.FieldByName(name)
		} else {
			continue
		}
		switch field.Type.Kind() {
		case reflect.Bool:
			destField.SetBool(value.Bool())
		case reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
			v := value.Int()
			if v > 0 {
				destField.SetInt(v)
			}
		case reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
			v := value.Uint()
			if v > 0 {
				destField.SetUint(v)
			}
		case reflect.Float32, reflect.Float64:
			v := value.Float()
			if v > 0.0 {
				destField.SetFloat(v)
			}
		case reflect.String:
			v := value.String()
			if len(v) == 0 {
				break
			}
			if name == Foundation || name == CRCAddress {
				t, err := common.Uint168FromAddress(v)
				if err == nil {
					arrayValue := reflect.ValueOf(t).Elem()
					destField.Set(arrayValue)
					if name == Foundation {
						block := config.GenesisBlock(t)
						if _, ok := paramsType.FieldByName(GenesisBlock); ok {
							blockValue := reflect.ValueOf(block)
							destField = paramsValue.FieldByName(GenesisBlock)
							destField.Set(blockValue)
						}
					}

				}
				break
			}
			destField.Set(value)
		case reflect.Slice:
			if !value.IsNil() {
				destField.Set(value)
			}
		case reflect.Ptr:
			if !value.IsNil() {
				destField.Set(value)
			}
		}

	}
}

