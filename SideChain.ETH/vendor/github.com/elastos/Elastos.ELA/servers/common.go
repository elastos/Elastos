package servers

import (
	"github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const TlsPort = 443

type AttributeInfo struct {
	Usage AttributeUsage `json:"usage"`
	Data  string         `json:"data"`
}

type InputInfo struct {
	TxID     string `json:"txid"`
	VOut     uint16 `json:"vout"`
	Sequence uint32 `json:"sequence"`
}

type OutputInfo struct {
	Value         string            `json:"value"`
	Index         uint32            `json:"n"`
	Address       string            `json:"address"`
	AssetID       string            `json:"assetid"`
	OutputLock    uint32            `json:"outputlock"`
	OutputType    uint32            `json:"type"`
	OutputPayload OutputPayloadInfo `json:"payload"`
}

type OutputPayloadInfo interface{}

type DefaultOutputInfo struct{}

type VoteContentInfo struct {
	VoteType       outputpayload.VoteType `json:"votetype"`
	CandidatesInfo []string               `json:"candidates"`
}

type VoteOutputInfo struct {
	Version  byte              `json:"version"`
	Contents []VoteContentInfo `json:"contents"`
}

type ProgramInfo struct {
	Code      string `json:"code"`
	Parameter string `json:"parameter"`
}

type TransactionInfo struct {
	TxID           string             `json:"txid"`
	Hash           string             `json:"hash"`
	Size           uint32             `json:"size"`
	VSize          uint32             `json:"vsize"`
	Version        TransactionVersion `json:"version"`
	LockTime       uint32             `json:"locktime"`
	Inputs         []InputInfo        `json:"vin"`
	Outputs        []OutputInfo       `json:"vout"`
	BlockHash      string             `json:"blockhash"`
	Confirmations  uint32             `json:"confirmations"`
	Time           uint32             `json:"time"`
	BlockTime      uint32             `json:"blocktime"`
	TxType         TxType             `json:"type"`
	PayloadVersion byte               `json:"payloadversion"`
	Payload        PayloadInfo        `json:"payload"`
	Attributes     []AttributeInfo    `json:"attributes"`
	Programs       []ProgramInfo      `json:"programs"`
}

type BlockInfo struct {
	Hash              string        `json:"hash"`
	Confirmations     uint32        `json:"confirmations"`
	StrippedSize      uint32        `json:"strippedsize"`
	Size              uint32        `json:"size"`
	Weight            uint32        `json:"weight"`
	Height            uint32        `json:"height"`
	Version           uint32        `json:"version"`
	VersionHex        string        `json:"versionhex"`
	MerkleRoot        string        `json:"merkleroot"`
	Tx                []interface{} `json:"tx"`
	Time              uint32        `json:"time"`
	MedianTime        uint32        `json:"mediantime"`
	Nonce             uint32        `json:"nonce"`
	Bits              uint32        `json:"bits"`
	Difficulty        string        `json:"difficulty"`
	ChainWork         string        `json:"chainwork"`
	PreviousBlockHash string        `json:"previousblockhash"`
	NextBlockHash     string        `json:"nextblockhash"`
	AuxPow            string        `json:"auxpow"`
	MinerInfo         string        `json:"minerinfo"`
}

type VoteInfo struct {
	Signer string `json:"signer"`
	Accept bool   `json:"accept"`
}

type ConfirmInfo struct {
	BlockHash  string     `json:"blockhash"`
	Sponsor    string     `json:"sponsor"`
	ViewOffset uint32     `json:"viewoffset"`
	Votes      []VoteInfo `json:"votes"`
}

type ServerInfo struct {
	Compile   string      `json:"compile"`   // The compile version of this server node
	Height    uint32      `json:"height"`    // The ServerNode latest block height
	Version   uint32      `json:"version"`   // The network protocol the ServerNode used
	Services  string      `json:"services"`  // The services the server supports
	Port      uint16      `json:"port"`      // The nodes's port
	RPCPort   uint16      `json:"rpcport"`   // The RPC service port
	RestPort  uint16      `json:"restport"`  // The RESTful service port
	WSPort    uint16      `json:"wsport"`    // The webservcie port
	Neighbors []*PeerInfo `json:"neighbors"` // The connected neighbor peers.
}

type PeerInfo struct {
	NetAddress     string `json:"netaddress"`
	Services       string `json:"services"`
	RelayTx        bool   `json:"relaytx"`
	LastSend       string `json:"lastsend"`
	LastRecv       string `json:"lastrecv"`
	ConnTime       string `json:"conntime"`
	TimeOffset     int64  `json:"timeoffset"`
	Version        uint32 `json:"version"`
	Inbound        bool   `json:"inbound"`
	StartingHeight uint32 `json:"startingheight"`
	LastBlock      uint32 `json:"lastblock"`
	LastPingTime   string `json:"lastpingtime"`
	LastPingMicros int64  `json:"lastpingmicros"`
}

type ArbitratorGroupInfo struct {
	OnDutyArbitratorIndex int      `json:"ondutyarbitratorindex"`
	Arbitrators           []string `json:"arbitrators"`
}

type PayloadInfo interface{}

type CoinbaseInfo struct {
	CoinbaseData string `json:"coinbasedata"`
}

type RegisterAssetInfo struct {
	Asset      payload.Asset `json:"asset"`
	Amount     string        `json:"amount"`
	Controller string        `json:"controller"`
}

type SideChainPowInfo struct {
	BlockHeight     uint32 `json:"blockheight"`
	SideBlockHash   string `json:"sideblockhash"`
	SideGenesisHash string `json:"sidegenesishash"`
	Signature       string `json:"signature"`
}

type TransferCrossChainAssetInfo struct {
	CrossChainAddresses []string         `json:"crosschainaddresses"`
	OutputIndexes       []uint64         `json:"outputindexes"`
	CrossChainAmounts   []common.Fixed64 `json:"crosschainamounts"`
}

type WithdrawFromSideChainInfo struct {
	BlockHeight                uint32   `json:"blockheight"`
	GenesisBlockAddress        string   `json:"genesisblockaddress"`
	SideChainTransactionHashes []string `json:"sidechaintransactionhashes"`
}

type ProducerInfo struct {
	OwnerPublicKey string `json:"ownerpublickey"`
	NodePublicKey  string `json:"nodepublickey"`
	NickName       string `json:"nickname"`
	Url            string `json:"url"`
	Location       uint64 `json:"location"`
	NetAddress     string `json:"netaddress"`
	Signature      string `json:"signature"`
}

type CancelProducerInfo struct {
	OwnerPublicKey string `json:"ownerpublickey"`
	Signature      string `json:"signature"`
}

type ActivateProducerInfo struct {
	NodePublicKey string `json:"nodepublickey"`
	Signature     string `json:"signature"`
}

type UTXOInfo struct {
	TxType        byte   `json:"txtype"`
	TxID          string `json:"txid"`
	AssetID       string `json:"assetid"`
	VOut          uint32 `json:"vout"`
	Address       string `json:"address"`
	Amount        string `json:"amount"`
	OutputLock    uint32 `json:"outputlock"`
	Confirmations uint32 `json:"confirmations"`
}

type SidechainIllegalDataInfo struct {
	IllegalType         uint8    `json:"illegaltype"`
	Height              uint32   `json:"height"`
	IllegalSigner       string   `json:"illegalsigner"`
	Evidence            string   `json:"evidence"`
	CompareEvidence     string   `json:"compareevidence"`
	GenesisBlockAddress string   `json:"genesisblockaddress"`
	Signs               []string `json:"signs"`
}
