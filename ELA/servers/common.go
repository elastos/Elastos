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
	OutputType    uint32            `json:"outputtype"`
	OutputPayload OutputPayloadInfo `json:"outputpayload"`
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
	TxType         TransactionType    `json:"type"`
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

type NodeState struct {
	Compile     string // The compile version of this server node
	ID          uint64 // The nodes's id
	HexID       string // The nodes's id in hex format
	Height      uint64 // The ServerNode latest block height
	Version     uint32 // The network protocol the ServerNode used
	Services    uint64 // The services the local node supplied
	Relay       bool   // The relay capability of the ServerNode (merge into capbility flag)
	TxnCnt      uint64 // The transactions be transmit by
	RxTxnCnt    uint64 // The transaction received by this ServerNode
	Port        uint16 // The nodes's port
	PRCPort     uint16 // The RPC service prot
	RestPort    uint16 // The RESTful service port
	WSPort      uint16 // The webservcie port
	OpenPort    uint16 // The open service port
	OpenService bool   // If open service is enabled
	Neighbors   []Neighbor
}

type Neighbor struct {
	ID         uint64 // The neighbor ID
	HexID      string // The neighbor ID in hex format
	Height     uint64 // The neighbor height
	Services   uint64 // The services the neighbor node supplied
	Relay      bool   // If this neighbor relay block and transactions
	External   bool   // If this neighbor is an external node
	State      string // The state of this neighbor node
	NetAddress string // The tcp address of this neighbor node
}

type ArbitratorGroupInfo struct {
	OnDutyArbitratorIndex int
	Arbitrators           []string
}

type PayloadInfo interface{}

type CoinbaseInfo struct {
	CoinbaseData string
}

type RegisterAssetInfo struct {
	Asset      payload.Asset
	Amount     string
	Controller string
}

type SideChainPowInfo struct {
	BlockHeight     uint32
	SideBlockHash   string
	SideGenesisHash string
	SignedData      string
}

type TransferCrossChainAssetInfo struct {
	CrossChainAddresses []string
	OutputIndexes       []uint64
	CrossChainAmounts   []common.Fixed64
}

type WithdrawFromSideChainInfo struct {
	BlockHeight                uint32
	GenesisBlockAddress        string
	SideChainTransactionHashes []string
}

type RegisterProducerInfo struct {
	PublicKey string `json:"publickey"`
	NickName  string `json:"nickname"`
	Url       string `json:"url"`
	Location  uint64 `json:"location"`
	Address   string `json:"address"`
	Signature string `json:"signature"`
}

type CancelProducerInfo struct {
	PublicKey string `json:"publickey"`
	Signature string `json:"signature"`
}

type UpdateProducerInfo struct {
	*RegisterProducerInfo
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
