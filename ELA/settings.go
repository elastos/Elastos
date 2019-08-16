// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"strconv"
	"strings"
	"time"

	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/gpath"

	"github.com/urfave/cli"
)

const (
	// dataPath indicates the path storing the chain data.
	dataPath = "data"

	// logPath indicates the path storing the node log.
	nodeLogPath = "logs/node"

	// checkpointPath indicates the path storing the checkpoint data
	checkpointPath = "checkpoints"

	// cmdValueSplitter defines the splitter to split raw string into a
	// string array
	cmdValueSplitter = ","
)

var (
	// defaultConfig defines the default parameters to running a ELA node.
	defaultConfig = config.Configuration{
		PrintLevel: elalog.LevelInfo,
	}
)

type settingItem struct {
	Flag         cli.Flag
	DefaultValue interface{}
	ConfigPath   string
	ParamName    string
	ConfigSetter func(string, *config.Params, *config.Configuration) error
	CliSetter    func(interface{}, *config.Params) error
}

func (s *settingItem) TryInitValue(params *config.Params,
	conf *config.Configuration, c *cli.Context) error {
	if s.Flag != nil && c.IsSet(s.Flag.GetName()) {
		value, err := s.getCliValue(c)
		if err != nil {
			return err
		}

		if s.CliSetter != nil {
			return s.CliSetter(value, params)
		} else {
			return gpath.Set(params, value, s.ParamName)
		}
	} else {
		needSet, err := s.notDefault(conf)
		if err != nil {
			return err
		}
		if needSet {
			return s.initByConfig(params, conf)
		}
	}
	return nil
}

func (s *settingItem) getCliValue(c *cli.Context) (interface{}, error) {
	value := c.String(s.Flag.GetName())
	switch s.DefaultValue.(type) {
	case common.Fixed64:
		v, err := strconv.ParseInt(value, 10, 64)
		if err != nil {
			return nil, err
		}
		return common.Fixed64(v), nil
	case time.Duration:
		v, err := strconv.ParseInt(value, 10, 64)
		if err != nil {
			return nil, err
		}
		return time.Duration(v), nil
	case uint16:
		v, err := strconv.ParseInt(value, 10, 16)
		if err != nil {
			return nil, err
		}
		return uint16(v), nil
	case uint32:
		v, err := strconv.ParseInt(value, 10, 32)
		if err != nil {
			return nil, err
		}
		return uint32(v), nil
	case int:
		v, err := strconv.ParseInt(value, 10, 32)
		if err != nil {
			return nil, err
		}
		return int(v), nil
	case int64:
		v, err := strconv.ParseInt(value, 10, 64)
		if err != nil {
			return nil, err
		}
		return v, nil
	case bool:
		v, err := strconv.ParseBool(value)
		if err != nil {
			return nil, err
		}
		return v, nil
	case string:
		return value, nil
	case []string:
		return strings.Split(value, cmdValueSplitter), nil
	default:
		return nil, errors.New("known value type")
	}
}

func (s *settingItem) notDefault(conf *config.Configuration) (bool, error) {
	value, err := gpath.At(conf, s.ConfigPath)
	if err != nil {
		return false, err
	}
	return !gpath.Equal(s.DefaultValue, value), nil
}

func (s *settingItem) initByConfig(params *config.Params,
	conf *config.Configuration) error {
	if s.ConfigSetter != nil {
		return s.ConfigSetter(s.ConfigPath, params, conf)
	} else {
		value, err := gpath.At(conf, s.ConfigPath)
		if err != nil {
			return err
		}
		return gpath.Set(params, value, s.ParamName)
	}
}

type settings struct {
	items   []settingItem
	conf    *config.Configuration
	params  *config.Params
	context *cli.Context
}

// Config return the loaded config parameters to running the ELA node.
func (s *settings) Config() *config.Configuration {
	return s.conf
}

// Params return a pointer to the parameters specific to the currently
// active ELA network.
func (s *settings) Params() *config.Params {
	return s.params
}

func (s *settings) SetContext(c *cli.Context) {
	s.context = c
}

func (s *settings) Flags() []cli.Flag {
	result := make([]cli.Flag, 0, len(s.items))
	for _, v := range s.items {
		if v.Flag != nil {
			result = append(result, v.Flag)
		}
	}
	return result
}

func (s *settings) InitParamsValue() {
	if err := s.initNetSetting(); err != nil {
		cmdcom.PrintErrorMsg(err.Error())
		os.Exit(1)
	}

	for _, v := range s.items {
		if err := v.TryInitValue(s.params, s.conf, s.context); err != nil {
			cmdcom.PrintErrorMsg(err.Error())
			os.Exit(1)
		}
	}
}

func (s *settings) Add(item *settingItem) {
	s.items = append(s.items, *item)
}

func (s *settings) initNetSetting() error {
	switch strings.ToLower(s.conf.ActiveNet) {
	case "testnet", "test":
		testNetDefault(s.conf)
		s.params = config.DefaultParams.TestNet()

	case "regnet", "reg":
		regNetDefault(s.conf)
		s.params = config.DefaultParams.RegNet()
		pact.MaxBlockSize = 2000000

	default:
		mainNetDefault(s.conf)
		s.params = &config.DefaultParams
		pact.MaxBlockSize = 2000000
	}
	if s.conf.MaxBlockSize > 0 {
		pact.MaxBlockSize = s.conf.MaxBlockSize
	}

	config.Parameters = s.conf
	if s.conf.PowConfiguration.InstantBlock {
		s.params = s.params.InstantBlock()
	}
	return nil
}

func newSettings() *settings {
	result := &settings{
		items: make([]settingItem, 0),
	}

	result.Add(&settingItem{
		Flag:         cmdcom.MagicFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "Magic",
		ParamName:    "Magic"})

	result.Add(&settingItem{
		Flag:         cmdcom.PortFlag,
		DefaultValue: uint16(0),
		ConfigPath:   "NodePort",
		ParamName:    "DefaultPort"})

	result.Add(&settingItem{
		Flag:         cmdcom.PeersFlag,
		DefaultValue: []string{},
		ConfigPath:   "PermanentPeers",
		ParamName:    "PermanentPeers"})

	result.Add(&settingItem{
		Flag:         cmdcom.DnsSeedFlag,
		DefaultValue: []string{},
		ConfigPath:   "DNSSeeds",
		ParamName:    "DNSSeeds"})

	result.Add(&settingItem{
		Flag:         cmdcom.EnableDnsFlag,
		DefaultValue: false,
		ConfigPath:   "DisableDNS",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			params.DNSSeeds = nil
			return nil
		},
		CliSetter: func(value interface{}, params *config.Params) error {
			disable, ok := value.(bool)
			if !ok {
				return errors.New("invalid dns seeds switch setting")
			}
			if disable {
				params.DNSSeeds = nil
			}
			return nil
		},
		ParamName: ""})

	result.Add(&settingItem{
		Flag:         cmdcom.MinTxFeeFlag,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "MinCrossChainTxFee",
		ParamName:    "MinCrossChainTxFee"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: "",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			foundation, err := common.Uint168FromAddress(conf.FoundationAddress)
			if err != nil {
				return errors.New("invalid foundation address")
			}
			params.Foundation = *foundation
			params.GenesisBlock = config.GenesisBlock(foundation)
			return nil
		},
		ConfigPath: "FoundationAddress",
		ParamName:  "Foundation"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: "",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			crcAddress, err := common.Uint168FromAddress(conf.CRCAddress)
			if err != nil {
				return errors.New("invalid CRC address")
			}
			params.CRCAddress = *crcAddress
			return nil
		},
		ConfigPath: "CRCAddress",
		ParamName:  "CRCAddress"})

	result.Add(&settingItem{
		Flag:         cmdcom.VoteStartHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "VoteStartHeight",
		ParamName:    "VoteStartHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CheckAddressHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CheckAddressHeight",
		ParamName:    "CheckAddressHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRCOnlyDPOSHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRCOnlyDPOSHeight",
		ParamName:    "CRCOnlyDPOSHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.PublicDPOSHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "PublicDPOSHeight",
		ParamName:    "PublicDPOSHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRCommitteeStartHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRCommitteeStartHeight",
		ParamName:    "CRCommitteeStartHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRVotingStartHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRVotingStartHeight",
		ParamName:    "CRVotingStartHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.EnableActivateIllegalHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "EnableActivateIllegalHeight",
		ParamName:    "EnableActivateIllegalHeight"})

	ckpManagerSetter := func(path string, params *config.Params,
		conf *config.Configuration) error {
		params.CkpManager = checkpoint.NewManager(&checkpoint.Config{
			EnableHistory:      conf.EnableHistory,
			HistoryStartHeight: conf.HistoryStartHeight,
		})
		return nil
	}
	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: false,
		ConfigSetter: ckpManagerSetter,
		ConfigPath:   "EnableHistory",
		ParamName:    ""})
	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: uint32(0),
		ConfigSetter: ckpManagerSetter,
		ConfigPath:   "HistoryStartHeight",
		ParamName:    ""})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: false,
		ConfigPath:   "EnableUtxoDB",
		ParamName:    "EnableUtxoDB"})

	// DPoS configurations

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: false,
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			// When arbiter service enabled, IP address must be set.
			return checkHost(conf.DPoSConfiguration.IPAddress)
		},
		ConfigPath: "DPoSConfiguration.EnableArbiter",
		ParamName:  ""})

	result.Add(&settingItem{
		Flag:         cmdcom.DPoSMagicFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "DPoSConfiguration.Magic",
		ParamName:    "DPoSMagic"})

	result.Add(&settingItem{
		Flag:         cmdcom.DPoSPortFlag,
		DefaultValue: uint16(0),
		ConfigPath:   "DPoSConfiguration.DPoSPort",
		ParamName:    "DPoSDefaultPort"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: []string{},
		ConfigPath:   "DPoSConfiguration.OriginArbiters",
		ParamName:    "OriginArbiters"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: []string{},
		ConfigPath:   "DPoSConfiguration.CRCArbiters",
		ParamName:    "CRCArbiters"})

	result.Add(&settingItem{
		Flag:         cmdcom.NormalArbitratorsCountFlag,
		DefaultValue: 0,
		ConfigPath:   "DPoSConfiguration.NormalArbitratorsCount",
		ParamName:    "GeneralArbiters"})

	result.Add(&settingItem{
		Flag:         cmdcom.PreConnectOffsetFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "DPoSConfiguration.PreConnectOffset",
		ParamName:    "PreConnectOffset"})

	result.Add(&settingItem{
		Flag:         cmdcom.CandidatesCountFlag,
		DefaultValue: 0,
		ConfigPath:   "DPoSConfiguration.CandidatesCount",
		ParamName:    "CandidateArbiters"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: time.Duration(0),
		ConfigPath:   "DPoSConfiguration.SignTolerance",
		ConfigSetter: func(s string, params *config.Params,
			conf *config.Configuration) error {
			params.ToleranceDuration =
				conf.DPoSConfiguration.SignTolerance * time.Second
			return nil
		},
		ParamName: "ToleranceDuration"})

	result.Add(&settingItem{
		Flag:         cmdcom.MaxInactiveRoundsFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "DPoSConfiguration.MaxInactiveRounds",
		ParamName:    "MaxInactiveRounds"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "DPoSConfiguration.InactivePenalty",
		ParamName:    "InactivePenalty"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "DPoSConfiguration.EmergencyInactivePenalty",
		ParamName:    "EmergencyInactivePenalty"})

	// CR configurations

	result.Add(&settingItem{
		Flag:         cmdcom.CRMemberCountFlag,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "CRConfiguration.MemberCount",
		ParamName:    "CRMemberCount"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRDutyPeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.DutyPeriod",
		ParamName:    "CRDutyPeriod"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRVotingPeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.VotingPeriod",
		ParamName:    "VotingPeriod"})

	return result
}

func (s *settings) SetupConfig() {
	configPath := s.context.String("conf")
	file, err := s.loadConfigFile(configPath)
	if err != nil {
		if s.context.IsSet("conf") {
			cmdcom.PrintErrorMsg(err.Error())
			os.Exit(1)
		}
		file = &defaultConfig
	}

	s.conf = file
}

// loadConfigFile read configuration parameters through the config file.
func (s *settings) loadConfigFile(path string) (*config.Configuration, error) {
	file, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	// Remove the UTF-8 Byte Order Mark
	file = bytes.TrimPrefix(file, []byte("\xef\xbb\xbf"))

	cfgFile := struct {
		config.Configuration `json:"Configuration"`
	}{
		Configuration: defaultConfig,
	}

	err = json.Unmarshal(file, &cfgFile)
	if err != nil {
		return nil, errors.New("config file parsing failed, " + err.Error())
	}

	return &cfgFile.Configuration, nil
}

// checkHost check the host or IP address is valid and available.
func checkHost(host string) error {
	// Empty host check.
	if host == "" {
		return errors.New("arbiter IPAddress must set when arbiter" +
			" service enabled")
	}

	// Skip if host is already an IP address.
	if ip := net.ParseIP(host); ip != nil {
		return nil
	}

	// Attempt to look up an IP address associated with the parsed host.
	ips, err := net.LookupIP(host)
	if err != nil {
		return err
	}
	if len(ips) == 0 {
		return fmt.Errorf("no addresses found for %s", host)
	}

	return nil
}

// mainNetDefault set the default parameters for main net usage.
func mainNetDefault(cfg *config.Configuration) {
	if cfg.HttpInfoPort == 0 {
		cfg.HttpInfoPort = 20333
	}
	if cfg.HttpRestPort == 0 {
		cfg.HttpRestPort = 20334
	}
	if cfg.HttpWsPort == 0 {
		cfg.HttpWsPort = 20335
	}
	if cfg.HttpJsonPort == 0 {
		cfg.HttpJsonPort = 20336
	}
}

// testNetDefault set the default parameters for test net usage.
func testNetDefault(cfg *config.Configuration) {
	if cfg.HttpInfoPort == 0 {
		cfg.HttpInfoPort = 21333
	}
	if cfg.HttpRestPort == 0 {
		cfg.HttpRestPort = 21334
	}
	if cfg.HttpWsPort == 0 {
		cfg.HttpWsPort = 21335
	}
	if cfg.HttpJsonPort == 0 {
		cfg.HttpJsonPort = 21336
	}
}

// regNetDefault set the default parameters for reg net usage.
func regNetDefault(cfg *config.Configuration) {
	if cfg.HttpInfoPort == 0 {
		cfg.HttpInfoPort = 22333
	}
	if cfg.HttpRestPort == 0 {
		cfg.HttpRestPort = 22334
	}
	if cfg.HttpWsPort == 0 {
		cfg.HttpWsPort = 22335
	}
	if cfg.HttpJsonPort == 0 {
		cfg.HttpJsonPort = 22336
	}
}
