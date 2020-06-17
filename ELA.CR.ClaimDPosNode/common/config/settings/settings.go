// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package settings

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
	// cmdValueSplitter defines the splitter to split raw string into a
	// string array
	cmdValueSplitter = ","

	// cmdNameSplitter defines the splitter to split raw string into a
	// string array
	commandNameSplitter = ","
)

var (
	// defaultConfig defines the default parameters to running a ELA node.
	defaultConfig = config.Configuration{
		PrintLevel: uint32(elalog.LevelInfo),
	}
)

type settingItem struct {
	Flag         cli.Flag
	DefaultValue interface{}
	ConfigPath   string
	ParamName    string
	ConfigSetter func(string, *config.Params, *config.Configuration) error
	CliSetter    func(interface{}, *config.Params, *config.Configuration) error
}

func (s *settingItem) TryInitValue(params *config.Params,
	conf *config.Configuration, c *cli.Context) error {

	if s.Flag != nil && c.IsSet(GetFullCommandName(s.Flag)) {
		value, err := s.getCliValue(c)
		if err != nil {
			return err
		}

		if s.CliSetter != nil {
			return s.CliSetter(value, params, conf)
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
	value := c.String(GetFullCommandName(s.Flag))
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
	case uint64:
		v, err := strconv.ParseUint(value, 10, 64)
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
		return nil, errors.New("unknown value type")
	}
}

func (s *settingItem) notDefault(conf *config.Configuration) (bool, error) {
	if len(s.ConfigPath) == 0 {
		return false, nil
	}
	value, err := gpath.At(conf, s.ConfigPath)
	if err != nil {
		return false, err
	}

	if gpath.IsNil(value) {
		return false, nil
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

type Settings struct {
	items   []settingItem
	conf    *config.Configuration
	params  *config.Params
	context *cli.Context
}

// Config return the loaded config parameters to running the ELA node.
func (s *Settings) Config() *config.Configuration {
	return s.conf
}

// Params return a pointer to the parameters specific to the currently
// active ELA network.
func (s *Settings) Params() *config.Params {
	return s.params
}

func (s *Settings) SetContext(c *cli.Context) {
	s.context = c
}

func (s *Settings) Flags() []cli.Flag {
	result := make([]cli.Flag, 0, len(s.items))
	for _, v := range s.items {
		if v.Flag != nil {
			result = append(result, v.Flag)
		}
	}
	return result
}

func (s *Settings) InitParamsValue() {
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

func (s *Settings) Add(item *settingItem) {
	s.items = append(s.items, *item)
}

func (s *Settings) initNetSetting() (err error) {
	var testNet, regTest bool
	switch strings.ToLower(s.conf.ActiveNet) {
	case "testnet", "test":
		testNet = true
	case "regnet", "reg":
		regTest = true
	default:
		testNet = false
		regTest = false
	}

	if s.context.IsSet(GetFullCommandName(cmdcom.TestNetFlag)) {
		if testNet, err = strconv.ParseBool(s.context.String(
			GetFullCommandName(cmdcom.TestNetFlag))); err != nil {
			return
		}
	} else if s.context.IsSet(GetFullCommandName(cmdcom.RegTestFlag)) {
		if regTest, err = strconv.ParseBool(s.context.String(
			GetFullCommandName(cmdcom.RegTestFlag))); err != nil {
			return
		}
	}

	if testNet {
		if err := s.testNetDefault(s.conf); err != nil {
			return err
		}
		s.params = config.DefaultParams.TestNet()
	} else if regTest {
		if err := s.regNetDefault(s.conf); err != nil {
			return err
		}
		s.params = config.DefaultParams.RegNet()
	} else {
		if err := s.mainNetDefault(s.conf); err != nil {
			return err
		}
		s.params = &config.DefaultParams
	}

	if s.conf.MaxBlockSize > 0 {
		pact.MaxBlockContextSize = s.conf.MaxBlockSize
	} else if !testNet {
		pact.MaxBlockContextSize = 2000000
	}

	if s.conf.MaxBlockHeaderSize > 0 {
		pact.MaxBlockHeaderSize = s.conf.MaxBlockHeaderSize
	}

	if s.conf.MaxTxPerBlock > 0 {
		pact.MaxTxPerBlock = s.conf.MaxTxPerBlock
	} else {
		pact.MaxTxPerBlock = 10000
	}

	config.Parameters = s.conf
	instantBlock := s.conf.PowConfiguration.InstantBlock
	if s.context.IsSet(GetFullCommandName(cmdcom.InstantBlockFlag)) {
		if instantBlock, err = strconv.ParseBool(s.context.String(
			GetFullCommandName(cmdcom.InstantBlockFlag))); err != nil {
			return
		}
	}
	if instantBlock {
		s.params = s.params.InstantBlock()
	}
	return
}

func NewSettings() *Settings {
	result := &Settings{
		items: make([]settingItem, 0),
	}

	result.Add(&settingItem{
		Flag:         cmdcom.MagicFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "Magic",
		ParamName:    "Magic"})

	result.Add(&settingItem{
		Flag:         cmdcom.PrintLevelFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "PrintLevel",
		ParamName:    "PrintLevel"})

	result.Add(&settingItem{
		Flag:         cmdcom.PortFlag,
		DefaultValue: uint16(0),
		ConfigPath:   "NodePort",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			if conf.NodePort > 0 {
				params.DefaultPort = conf.NodePort
			}
			return nil
		},
		ParamName: "DefaultPort"})

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
		Flag:         cmdcom.AccountWalletFlag,
		DefaultValue: "",
		ConfigPath:   "WalletPath",
		ParamName:    "WalletPath"})

	result.Add(&settingItem{
		Flag:         cmdcom.EnableDnsFlag,
		DefaultValue: false,
		ConfigPath:   "DisableDNS",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			params.DNSSeeds = nil
			return nil
		},
		CliSetter: func(value interface{}, params *config.Params,
			conf *config.Configuration) error {
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

	result.Add(&settingItem{Flag: cmdcom.MinTxFeeFlag,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "MinCrossChainTxFee",
		ParamName:    "MinCrossChainTxFee"})

	initFoundation := func(addr string, params *config.Params) error {
		foundation, err := common.Uint168FromAddress(addr)
		if err != nil {
			return errors.New("invalid foundation address")
		}
		params.Foundation = *foundation
		params.GenesisBlock = config.GenesisBlock(foundation)
		return nil
	}
	result.Add(&settingItem{
		Flag:         cmdcom.FoundationAddrFlag,
		DefaultValue: "",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			return initFoundation(conf.FoundationAddress, params)
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			value, ok := i.(string)
			if !ok {
				return errors.New("unknown foundation address type")
			}
			return initFoundation(value, params)
		},
		ConfigPath: "FoundationAddress",
		ParamName:  "Foundation"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: "",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			crcAddress, err := common.Uint168FromAddress(conf.CRConfiguration.CRCAddress)
			if err != nil {
				return errors.New("invalid CRC address")
			}
			params.CRCAddress = *crcAddress
			return nil
		},
		ConfigPath: "CRConfiguration.CRCAddress",
		ParamName:  "CRCAddress"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRAssetsAddressFlag,
		DefaultValue: "",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			crAssetsAddress, err := common.Uint168FromAddress(conf.CRConfiguration.CRAssetsAddress)
			if err != nil {
				return errors.New("invalid CR assets address")
			}
			params.CRAssetsAddress = *crAssetsAddress
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			value, ok := i.(string)
			if !ok {
				return errors.New("unknown foundation address type")
			}
			crAssetsAddress, err := common.Uint168FromAddress(value)
			if err != nil {
				return errors.New("invalid CR assets address")
			}
			params.CRAssetsAddress = *crAssetsAddress
			return nil
		},
		ConfigPath: "CRConfiguration.CRAssetsAddress",
		ParamName:  "CRAssetsAddress"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRExpensesAddressFlag,
		DefaultValue: "",
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			CRExpensesAddress, err := common.Uint168FromAddress(conf.CRConfiguration.CRExpensesAddress)
			if err != nil {
				return errors.New("invalid CR expenses address")
			}
			params.CRExpensesAddress = *CRExpensesAddress
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			value, ok := i.(string)
			if !ok {
				return errors.New("unknown foundation address type")
			}
			CRExpensesAddress, err := common.Uint168FromAddress(value)
			if err != nil {
				return errors.New("invalid CR expenses address")
			}
			params.CRExpensesAddress = *CRExpensesAddress
			return nil
		},
		ConfigPath: "CRConfiguration.CRExpensesAddress",
		ParamName:  "CRExpensesAddress"})

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
		Flag:         cmdcom.CheckRewardHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CheckRewardHeight",
		ParamName:    "CheckRewardHeight"})

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
		ConfigPath:   "CRConfiguration.CRCommitteeStartHeight",
		ParamName:    "CRCommitteeStartHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRClaimDPOSNodeStartHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.CRClaimDPOSNodeStartHeight",
		ParamName:    "CRClaimDPOSNodeStartHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRClaimDPOSNodePeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.CRClaimDPOSNodePeriod",
		ParamName:    "CRClaimDPOSNodePeriod"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRVotingStartHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.CRVotingStartHeight",
		ParamName:    "CRVotingStartHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.MaxCommitteeProposalCount,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.MaxCommitteeProposalCount",
		ParamName:    "MaxCommitteeProposalCount"})

	result.Add(&settingItem{
		Flag:         cmdcom.MaxNodePerHost,
		DefaultValue: uint32(0),
		ConfigPath:   "MaxNodePerHost",
		ParamName:    "MaxNodePerHost"})

	result.Add(&settingItem{
		Flag:         cmdcom.VoteStatisticsHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "VoteStatisticsHeight",
		ParamName:    "VoteStatisticsHeight"})

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
			NeedSave:           false,
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

	result.Add(&settingItem{
		Flag:         cmdcom.AutoMiningFlag,
		DefaultValue: false,
		ConfigSetter: func(string, *config.Params,
			*config.Configuration) error {
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			mining, ok := i.(bool)
			if !ok {
				return errors.New("invalid auto mining value")
			}
			conf.PowConfiguration.AutoMining = mining
			return nil
		}})

	result.Add(&settingItem{
		Flag:         cmdcom.PayToAddrFlag,
		DefaultValue: "",
		ConfigSetter: func(string, *config.Params,
			*config.Configuration) error {
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			addr, ok := i.(string)
			if !ok {
				return errors.New("invalid pay to address value")
			}
			conf.PowConfiguration.PayToAddr = addr
			return nil
		}})

	result.Add(&settingItem{
		Flag:         cmdcom.EnableRPCFlag,
		DefaultValue: true,
		ConfigSetter: func(string, *config.Params,
			*config.Configuration) error {
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			enable, ok := i.(bool)
			if !ok {
				return errors.New("invalid enable rpc value")
			}
			conf.EnableRPC = enable
			return nil
		}})

	result.Add(&settingItem{
		Flag:         cmdcom.RPCUserFlag,
		DefaultValue: "",
		ConfigSetter: func(string, *config.Params,
			*config.Configuration) error {
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			user, ok := i.(string)
			if !ok {
				return errors.New("invalid rpc user value")
			}
			conf.RpcConfiguration.User = user
			return nil
		}})

	result.Add(&settingItem{
		Flag:         cmdcom.RPCPasswordFlag,
		DefaultValue: "",
		ConfigSetter: func(string, *config.Params,
			*config.Configuration) error {
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			pass, ok := i.(string)
			if !ok {
				return errors.New("invalid rpc password value")
			}
			conf.RpcConfiguration.Pass = pass
			return nil
		}})

	result.Add(&settingItem{
		Flag:         cmdcom.RPCAllowedIPsFlag,
		DefaultValue: []string{},
		ConfigSetter: func(string, *config.Params,
			*config.Configuration) error {
			return nil
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			ipStr, ok := i.([]string)
			if !ok {
				return errors.New("invalid ip list")
			}
			conf.RpcConfiguration.WhiteIPList = ipStr
			return nil
		}})

	// DPoS configurations

	result.Add(&settingItem{
		Flag:         cmdcom.EnableArbiterFlag,
		DefaultValue: false,
		ConfigSetter: func(path string, params *config.Params,
			conf *config.Configuration) error {
			// When arbiter service enabled, IP address must be set.
			return checkHost(conf.DPoSConfiguration.IPAddress)
		},
		CliSetter: func(i interface{}, params *config.Params,
			conf *config.Configuration) error {
			enable, ok := i.(bool)
			if !ok {
				return errors.New("invalid enable arbiter value")
			}
			conf.DPoSConfiguration.EnableArbiter = enable
			return nil
		},
		ConfigPath: "DPoSConfiguration.EnableArbiter",
		ParamName:  ""})

	result.Add(&settingItem{
		Flag:         cmdcom.DPoSMagicFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "DPoSConfiguration.Magic",
		ParamName:    "DPoSMagic"})

	result.Add(&settingItem{
		Flag:         cmdcom.DPoSIPAddressFlag,
		DefaultValue: "",
		ConfigPath:   "DPoSConfiguration.IPAddress",
		ParamName:    "DPoSIPAddress"})

	result.Add(&settingItem{
		Flag:         cmdcom.DPoSPortFlag,
		DefaultValue: uint16(0),
		ConfigPath:   "DPoSConfiguration.DPoSPort",
		ParamName:    "DPoSDefaultPort"})

	result.Add(&settingItem{
		Flag:         cmdcom.OriginArbitersFlag,
		DefaultValue: []string{},
		ConfigPath:   "DPoSConfiguration.OriginArbiters",
		ParamName:    "OriginArbiters"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRCArbitersFlag,
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
		Flag:         cmdcom.InactivePenaltyFlag,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "DPoSConfiguration.InactivePenalty",
		ParamName:    "InactivePenalty"})

	result.Add(&settingItem{
		Flag:         cmdcom.EmergencyInactivePenaltyFlag,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "DPoSConfiguration.EmergencyInactivePenalty",
		ParamName:    "EmergencyInactivePenalty"})

	// CR configurations

	result.Add(&settingItem{
		Flag:         cmdcom.CRMemberCountFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.MemberCount",
		ParamName:    "CRMemberCount"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRDutyPeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.DutyPeriod",
		ParamName:    "CRDutyPeriod"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRDepositLockupBlocksFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.DepositLockupBlocks",
		ParamName:    "CRDepositLockupBlocks"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRVotingPeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.VotingPeriod",
		ParamName:    "CRVotingPeriod"})

	result.Add(&settingItem{
		Flag:         cmdcom.ProposalCRVotingPeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.ProposalCRVotingPeriod",
		ParamName:    "ProposalCRVotingPeriod"})

	result.Add(&settingItem{
		Flag:         cmdcom.ProposalPublicVotingPeriodFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.ProposalPublicVotingPeriod",
		ParamName:    "ProposalPublicVotingPeriod"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRAgreementCountFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.CRAgreementCount",
		ParamName:    "CRAgreementCount"})

	result.Add(&settingItem{
		Flag:         cmdcom.VoterRejectPercentageFlag,
		DefaultValue: float64(0),
		ConfigPath:   "CRConfiguration.VoterRejectPercentage",
		ParamName:    "VoterRejectPercentage"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRCAppropriatePercentageFlag,
		DefaultValue: float64(0),
		ConfigPath:   "CRConfiguration.CRCAppropriatePercentage",
		ParamName:    "CRCAppropriatePercentage"})

	result.Add(&settingItem{
		Flag:         cmdcom.SecretaryGeneralFlag,
		DefaultValue: "",
		ConfigPath:   "CRConfiguration.SecretaryGeneral",
		ParamName:    "SecretaryGeneral"})

	result.Add(&settingItem{
		Flag:         cmdcom.MaxProposalTrackingCountFlag,
		DefaultValue: uint8(0),
		ConfigPath:   "CRConfiguration.MaxProposalTrackingCount",
		ParamName:    "MaxProposalTrackingCount"})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: "",
		ConfigPath:   "RPCServiceLevel",
		ParamName:    "RPCServiceLevel",
	})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: "",
		ConfigPath:   "NodeProfileStrategy",
		ParamName:    "NodeProfileStrategy",
	})

	result.Add(&settingItem{
		Flag:         nil,
		DefaultValue: uint32(0),
		ConfigPath:   "TxCacheVolume",
		ParamName:    "TxCacheVolume",
	})

	result.Add(&settingItem{
		Flag:         cmdcom.RegisterCRByDIDHeightFlag,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.RegisterCRByDIDHeight",
		ParamName:    "RegisterCRByDIDHeight",
	})

	result.Add(&settingItem{
		Flag:         cmdcom.MaxCRAssetsAddressUTXOCount,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.MaxCRAssetsAddressUTXOCount",
		ParamName:    "MaxCRAssetsAddressUTXOCount"})

	result.Add(&settingItem{
		Flag:         cmdcom.MinCRAssetsAddressUTXOCount,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.MinCRAssetsAddressUTXOCount",
		ParamName:    "MinCRAssetsAddressUTXOCount"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRAssetsRectifyTransactionHeight,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.CRAssetsRectifyTransactionHeight",
		ParamName:    "CRAssetsRectifyTransactionHeight"})

	result.Add(&settingItem{
		Flag:         cmdcom.CRCProposalWithdrawPayloadV1Height,
		DefaultValue: uint32(0),
		ConfigPath:   "CRConfiguration.CRCProposalWithdrawPayloadV1Height",
		ParamName:    "CRCProposalWithdrawPayloadV1Height"})

	result.Add(&settingItem{
		Flag:         cmdcom.RectifyTxFee,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "CRConfiguration.RectifyTxFee",
		ParamName:    "RectifyTxFee"})

	result.Add(&settingItem{
		Flag:         cmdcom.RealWithdrawSingleFee,
		DefaultValue: common.Fixed64(0),
		ConfigPath:   "CRConfiguration.RealWithdrawSingleFee",
		ParamName:    "RealWithdrawSingleFee"})

	result.Add(&settingItem{
		Flag:         cmdcom.NewVersionHeight,
		DefaultValue: uint64(0),
		ConfigPath:   "CRConfiguration.NewVersionHeight",
		ParamName:    "NewVersionHeight"})

	return result
}

func (s *Settings) SetupConfig() {
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
func (s *Settings) loadConfigFile(path string) (*config.Configuration, error) {
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
func (s *Settings) mainNetDefault(cfg *config.Configuration) error {
	if err := s.trySetUintPortValue(cmdcom.InfoPortFlag.Name, &cfg.HttpInfoPort,
		20333); err != nil {
		return err
	}
	if err := s.trySetPortValue(cmdcom.RestPortFlag.Name, &cfg.HttpRestPort,
		20334); err != nil {
		return err
	}
	if err := s.trySetPortValue(cmdcom.WsPortFlag.Name, &cfg.HttpWsPort,
		20335); err != nil {
		return err
	}
	return s.trySetPortValue(cmdcom.RPCPortFlag.Name,
		&cfg.HttpJsonPort, 20336)
}

// testNetDefault set the default parameters for test net usage.
func (s *Settings) testNetDefault(cfg *config.Configuration) error {
	if err := s.trySetUintPortValue(cmdcom.InfoPortFlag.Name, &cfg.HttpInfoPort,
		21333); err != nil {
		return err
	}
	if err := s.trySetPortValue(cmdcom.RestPortFlag.Name, &cfg.HttpRestPort,
		21334); err != nil {
		return err
	}
	if err := s.trySetPortValue(cmdcom.WsPortFlag.Name, &cfg.HttpWsPort,
		21335); err != nil {
		return err
	}
	return s.trySetPortValue(cmdcom.RPCPortFlag.Name,
		&cfg.HttpJsonPort, 21336)
}

// regNetDefault set the default parameters for reg net usage.
func (s *Settings) regNetDefault(cfg *config.Configuration) error {
	if err := s.trySetUintPortValue(cmdcom.InfoPortFlag.Name, &cfg.HttpInfoPort,
		22333); err != nil {
		return err
	}
	if err := s.trySetPortValue(cmdcom.RestPortFlag.Name, &cfg.HttpRestPort,
		22334); err != nil {
		return err
	}
	if err := s.trySetPortValue(cmdcom.WsPortFlag.Name, &cfg.HttpWsPort,
		22335); err != nil {
		return err
	}
	return s.trySetPortValue(cmdcom.RPCPortFlag.Name,
		&cfg.HttpJsonPort, 22336)
}

func (s *Settings) trySetPortValue(cliFlag string, value *int,
	defaultValue int) error {
	if s.context.IsSet(cliFlag) {
		info, err := strconv.ParseInt(s.context.String(
			cliFlag), 10, 32)
		if err != nil {
			return err
		}
		*value = int(info)
	} else if *value == 0 {
		*value = defaultValue
	}
	return nil
}

func (s *Settings) trySetUintPortValue(cliFlag string, value *uint16,
	defaultValue uint16) error {
	if s.context.IsSet(cliFlag) {
		info, err := strconv.ParseUint(s.context.String(
			cliFlag), 10, 16)
		if err != nil {
			return err
		}
		*value = uint16(info)
	} else if *value == 0 {
		*value = defaultValue
	}
	return nil
}

func GetFullCommandName(flag cli.Flag) string {
	result := strings.Split(flag.GetName(), commandNameSplitter)[0]
	return strings.TrimSpace(result)
}
