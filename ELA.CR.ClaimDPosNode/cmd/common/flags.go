// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package common

import (
	"errors"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/utils"

	"github.com/urfave/cli"
)

var (
	// Account flags
	AccountWalletFlag = cli.StringFlag{
		Name:  "wallet, w",
		Usage: "wallet `<file>` path",
		Value: account.KeystoreFileName,
	}
	AccountPasswordFlag = cli.StringFlag{
		Name:  "password, p",
		Usage: "wallet password",
	}
	AccountMultiMFlag = cli.IntFlag{
		Name:  "m",
		Usage: "min signature `<number>` of multi signature address",
	}
	AccountMultiPubKeyFlag = cli.StringFlag{
		Name:  "pubkeys, pks",
		Usage: "public key list of multi signature address, separate public keys with comma `,`",
	}

	// Transaction flags
	TransactionFromFlag = cli.StringFlag{
		Name:  "from",
		Usage: "the sender `<address>` of the transaction",
	}
	TransactionToFlag = cli.StringFlag{
		Name:  "to",
		Usage: "the recipient `<address>` of the transaction",
	}
	TransactionToManyFlag = cli.StringFlag{
		Name:  "tomany",
		Usage: "the `<file>` path that contains multi-recipients and amount",
	}
	TransactionAmountFlag = cli.StringFlag{
		Name:  "amount",
		Usage: "the transfer `<amount>` of the transaction",
	}
	TransactionFeeFlag = cli.StringFlag{
		Name:  "fee",
		Usage: "the transfer `<fee>` of the transaction",
	}
	TransactionOutputLockFlag = cli.StringFlag{
		Name:  "outputlock",
		Usage: "the `<lock height>` to specify when the received asset can be spent",
	}
	TransactionTxLockFlag = cli.StringFlag{
		Name:  "txlock",
		Usage: "the `<lock height>` to specify when the transaction can be packaged",
	}
	TransactionHexFlag = cli.StringFlag{
		Name:  "hex",
		Usage: "the transaction content in hex string format to be sign or send",
	}
	TransactionFileFlag = cli.StringFlag{
		Name:  "file, f",
		Usage: "the file path to specify a transaction file path with the hex string content to be sign",
	}
	TransactionNodePublicKeyFlag = cli.StringFlag{
		Name:  "nodepublickey",
		Usage: "the node public key of an arbitrator which have been inactivated",
	}
	TransactionForFlag = cli.StringFlag{
		Name:  "for",
		Usage: "the `<file>` path that holds the list of candidates",
	}
	TransactionSAddressFlag = cli.StringFlag{
		Name:  "saddress",
		Usage: "the locked `<address>` on main chain represents one side chain",
	}

	// RPC flags
	RPCUserFlag = cli.StringFlag{
		Name:  "rpcuser",
		Usage: "username for JSON-RPC connections",
	}
	RPCPasswordFlag = cli.StringFlag{
		Name:  "rpcpassword",
		Usage: "password for JSON-RPC connections",
	}
	RPCPortFlag = cli.StringFlag{
		Name:  "rpcport",
		Usage: "JSON-RPC server listening port `<number>`",
	}
	EnableRPCFlag = cli.StringFlag{
		Name:  "server",
		Usage: "decide if open JSON-RPC server or not",
	}
	RPCAllowedIPsFlag = cli.StringFlag{
		Name:  "rpcips",
		Usage: "white IP list allowed to access RPC server",
	}

	// Info flags
	InfoStartFlag = cli.IntFlag{
		Name:  "start",
		Usage: "the start index of producers",
		Value: 0,
	}
	InfoLimitFlag = cli.Int64Flag{
		Name:  "limit",
		Usage: "the limit count of producers",
		Value: -1,
	}
	InfoProducerStateFlag = cli.StringFlag{
		Name:  "state",
		Usage: "the producer state you want",
	}

	// Config flags
	TestNetFlag = cli.StringFlag{
		Name:  "testnet",
		Usage: "specify network type to test net",
		Value: defaultConfigPath,
	}
	RegTestFlag = cli.StringFlag{
		Name:  "regtest",
		Usage: "specify network type to reg test net",
		Value: defaultConfigPath,
	}
	ConfigFileFlag = cli.StringFlag{
		Name:  "conf",
		Usage: "config `<file>` path, ",
		Value: defaultConfigPath,
	}
	DataDirFlag = cli.StringFlag{
		Name:  "datadir",
		Usage: "block data and logs storage `<path>`",
		Value: defaultDataDir,
	}
	MagicFlag = cli.StringFlag{
		Name:  "magic",
		Usage: "magic number for node to initialize p2p connection",
	}
	EnableDnsFlag = cli.StringFlag{
		Name:  "dnsseed",
		Usage: "enable dns seeds for node to initialize p2p connection",
	}
	DnsSeedFlag = cli.StringFlag{
		Name:  "dns",
		Usage: "dns seeds for node to initialize p2p connection",
	}
	PeersFlag = cli.StringFlag{
		Name:  "peers",
		Usage: "peers seeds for node to initialize p2p connection",
	}
	PortFlag = cli.StringFlag{
		Name:  "port",
		Usage: "default peer-to-peer port for the network",
	}
	InfoPortFlag = cli.StringFlag{
		Name:  "infoport",
		Usage: "port for the http info server",
	}
	RestPortFlag = cli.StringFlag{
		Name:  "restport",
		Usage: "port for the http restful server",
	}
	WsPortFlag = cli.StringFlag{
		Name:  "wsport",
		Usage: "port for the http web socket server",
	}
	InstantBlockFlag = cli.StringFlag{
		Name:  "instant",
		Usage: "specify if need to generate instant block",
	}
	PayToAddrFlag = cli.StringFlag{
		Name:  "paytoaddr",
		Usage: "specify the miner reward address",
	}
	AutoMiningFlag = cli.StringFlag{
		Name:  "automining",
		Usage: "specify if should open auto mining",
	}
	MinTxFeeFlag = cli.StringFlag{
		Name:  "mintxfee",
		Usage: "specify minimum transaction fee",
	}
	VoteStartHeightFlag = cli.StringFlag{
		Name: "votestartheight",
		Usage: "indicates the height of starting register producer and " +
			"vote related",
	}
	CheckAddressHeightFlag = cli.StringFlag{
		Name:  "checkaddressheight",
		Usage: "defines the height begin to check output hash",
	}
	CRCOnlyDPOSHeightFlag = cli.StringFlag{
		Name: "crconlydposheight",
		Usage: "(H1) indicates the height of DPOS consensus begins with only " +
			"CRC producers participate in producing block",
	}
	PublicDPOSHeightFlag = cli.StringFlag{
		Name: "publicdposheight",
		Usage: "(H2) indicates the height when public registered and elected " +
			"producers participate in DPOS consensus",
	}
	CRCommitteeStartHeightFlag = cli.StringFlag{
		Name:  "crcommitteestartheight",
		Usage: "defines the height of CR Committee started",
	}
	CRVotingStartHeightFlag = cli.StringFlag{
		Name:  "crvotingstartheight",
		Usage: "defines the height of CR voting started",
	}
	CheckRewardHeightFlag = cli.StringFlag{
		Name:  "checkrewardheight",
		Usage: "defines the height to check reward in block",
	}
	VoteStatisticsHeightFlag = cli.StringFlag{
		Name:  "votestatisticsheight",
		Usage: "defines the height to fix vote statistics error",
	}
	EnableActivateIllegalHeightFlag = cli.StringFlag{
		Name: "enableactivateillegalheight",
		Usage: "defines the start height to enable activate illegal producer" +
			" though activate tx",
	}
	DPoSMagicFlag = cli.StringFlag{
		Name:  "dposmagic",
		Usage: "defines the magic number used in the DPoS network",
	}
	DPoSIPAddressFlag = cli.StringFlag{
		Name:  "dposipaddress",
		Usage: "defines the default IP address for the DPoS network",
	}
	DPoSPortFlag = cli.StringFlag{
		Name:  "dposport",
		Usage: "defines the default port for the DPoS network",
	}
	PreConnectOffsetFlag = cli.StringFlag{
		Name:  "preconnectoffset",
		Usage: "defines the offset blocks to pre-connect to the block producers",
	}
	NormalArbitratorsCountFlag = cli.StringFlag{
		Name:  "normalarbitratorscount",
		Usage: "defines the number of general(no-CRC) arbiters",
	}
	CandidatesCountFlag = cli.StringFlag{
		Name:  "candidatescount",
		Usage: "defines the number of needed candidate arbiters",
	}
	MaxInactiveRoundsFlag = cli.StringFlag{
		Name:  "maxinactiverounds",
		Usage: "defines the maximum inactive rounds before producer takes penalty",
	}
	CRMemberCountFlag = cli.StringFlag{
		Name:  "crmembercount",
		Usage: "defines the number of CR committee members",
	}
	CRDutyPeriodFlag = cli.StringFlag{
		Name: "crdutyperiod",
		Usage: "defines the duration of a normal duty period which measured " +
			"by block height",
	}
	CRVotingPeriodFlag = cli.StringFlag{
		Name: "crvotingperiod",
		Usage: "defines the duration of voting period which measured by " +
			"block height",
	}
)

// MoveRPCFlags finds the rpc argument and moves it to the front
// of the argument array.
func MoveRPCFlags(args []string) ([]string, error) {
	newArgs := args[:1]
	cacheArgs := make([]string, 0)

	for i := 1; i < len(args); i++ {
		switch args[i] {
		case "--rpcport":
			fallthrough
		case "--rpcuser":
			fallthrough
		case "--rpcpassword":
			newArgs = append(newArgs, args[i])
			if i == len(args)-1 {
				return nil, errors.New("invalid flag " + args[i])
			}
			newArgs = append(newArgs, args[i+1])
			i++
		default:
			cacheArgs = append(cacheArgs, args[i])
		}
	}

	newArgs = append(newArgs, cacheArgs...)
	return newArgs, nil
}

// GetFlagPassword gets node's wallet password from command line or user input
func GetFlagPassword(c *cli.Context) ([]byte, error) {
	flagPassword := c.String("password")
	password := []byte(flagPassword)
	if flagPassword == "" {
		return utils.GetPassword()
	}

	return password, nil
}
