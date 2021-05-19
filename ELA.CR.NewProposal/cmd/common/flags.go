// Copyright (c) 2017-2020 The Elastos Foundation
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
	PrintLevelFlag = cli.StringFlag{
		Name:  "printlevel",
		Usage: "level to print log",
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
	FoundationAddrFlag = cli.StringFlag{
		Name:  "foundation",
		Usage: "specify the foundation address",
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
	CheckRewardHeightFlag = cli.StringFlag{
		Name:  "checkrewardheight",
		Usage: "defines the height begin to check reward",
	}
	EnableArbiterFlag = cli.StringFlag{
		Name:  "arbiter",
		Usage: "indicates where or not to enable DPoS arbiter switch",
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
	MaxCommitteeProposalCount = cli.StringFlag{
		Name:  "maxcommitteeproposalcount",
		Usage: "defines max count of the proposal that one cr can proposal",
	}
	MaxNodePerHost = cli.StringFlag{
		Name:  "maxnodeperhost",
		Usage: "defines max nodes that one host can establish",
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
	SecretaryGeneralFlag = cli.StringFlag{
		Name:  "secretarygeneral",
		Usage: "defines the secretary general of CR",
	}
	MaxProposalTrackingCountFlag = cli.StringFlag{
		Name:  "maxproposaltrackingcount",
		Usage: "defines the max count of CRC proposal tracking",
	}
	OriginArbitersFlag = cli.StringFlag{
		Name:  "originarbiters",
		Usage: "defines origin arbiters",
	}
	CRCArbitersFlag = cli.StringFlag{
		Name:  "crcarbiters",
		Usage: "defines crc arbiters",
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
	InactivePenaltyFlag = cli.StringFlag{
		Name:  "inactivepenalty",
		Usage: "defines penalty of inactive",
	}
	EmergencyInactivePenaltyFlag = cli.StringFlag{
		Name:  "emergencyinactivepenalty",
		Usage: "defines penalty of emergency inactive",
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
	CRDepositLockupBlocksFlag = cli.StringFlag{
		Name:  "crdepositlockupblocks",
		Usage: "DepositLockupBlocks indicates how many blocks need to wait when cancel",
	}
	CRVotingPeriodFlag = cli.StringFlag{
		Name: "crvotingperiod",
		Usage: "defines the duration of voting period which measured by " +
			"block height",
	}
	ProposalCRVotingPeriodFlag = cli.StringFlag{
		Name:  "proposalcrvotingperiod",
		Usage: "defines the duration of CR voting about a proposal",
	}
	ProposalPublicVotingPeriodFlag = cli.StringFlag{
		Name: "proposalpublicvotingperiod",
		Usage: "defines the duration of all voters send reject vote about " +
			"a proposal",
	}
	CRAgreementCountFlag = cli.StringFlag{
		Name: "cragreementcount",
		Usage: "defines minimum count to let a registered proposal transfer " +
			"to CRAgreed state",
	}
	VoterRejectPercentageFlag = cli.StringFlag{
		Name:  "voterrejectpercentage",
		Usage: "defines percentage about voters reject a proposal",
	}
	CRCProposalHashFlag = cli.StringFlag{
		Name:  "proposalhash",
		Usage: "the `<proposalhash>` of the transaction",
	}
	CRCProposalStageFlag = cli.StringFlag{
		Name:  "stage",
		Usage: "the  `<stage>` of the proposal",
	}
	CRCCommiteeAddrFlag = cli.StringFlag{
		Name:  "crccommiteeaddr",
		Usage: "the  `<crccommiteeaddr>`",
	}
	CRCAppropriatePercentageFlag = cli.StringFlag{
		Name:  "crcappropriatepercentage",
		Usage: "defines percentage about CRC appropriation",
	}
	CRAssetsAddressFlag = cli.StringFlag{
		Name:  "crassetsaddress",
		Usage: "defines foundation address of CRC",
	}
	CRExpensesAddressFlag = cli.StringFlag{
		Name:  "crexpensesaddress",
		Usage: "defines appropriation address of CRC committee",
	}
	RegisterCRByDIDHeightFlag = cli.StringFlag{
		Name:  "registercrbydidheight",
		Usage: "defines the height to support register CR by CID",
	}
	MaxCRAssetsAddressUTXOCount = cli.StringFlag{
		Name:  "maxcrassetsaddressutxocount",
		Usage: "defines the maximum number of utxo cr assets address can have ",
	}
	MinCRAssetsAddressUTXOCount = cli.StringFlag{
		Name:  "mincrassetsaddressutxocount",
		Usage: "defines the minimum number of utxo cr assets address can rectify",
	}
	CRAssetsRectifyTransactionHeight = cli.StringFlag{
		Name:  "crassetsrectifytransactionheight",
		Usage: "defines the cr rectify transaction start height",
	}
	CRCProposalWithdrawPayloadV1Height = cli.StringFlag{
		Name:  "crcproposalwithdrawpayloadv1height",
		Usage: "defines the crc withdraw proposal payload type v1 accept height",
	}
	RectifyTxFee = cli.StringFlag{
		Name:  "rectifytxfee",
		Usage: "defines the fee of cr rectify transaction",
	}
	RealWithdrawSingleFee = cli.StringFlag{
		Name:  "realwithdrawsinglefee",
		Usage: "defines the single fee of cr real proposal withdraw transaction",
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
