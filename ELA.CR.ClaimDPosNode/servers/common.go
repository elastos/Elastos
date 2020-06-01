// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

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

type RpcOutputInfo struct {
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

type CandidateVotes struct {
	Candidate string `json:"candidate"`
	Votes     string `json:"votes"`
}

type VoteContentInfo struct {
	VoteType       outputpayload.VoteType `json:"votetype"`
	CandidatesInfo []CandidateVotes       `json:"candidates"`
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
	TxType         TxType             `json:"type"`
	PayloadVersion byte               `json:"payloadversion"`
	Payload        PayloadInfo        `json:"payload"`
	Attributes     []AttributeInfo    `json:"attributes"`
	Inputs         []InputInfo        `json:"vin"`
	Outputs        []RpcOutputInfo    `json:"vout"`
	LockTime       uint32             `json:"locktime"`
	Programs       []ProgramInfo      `json:"programs"`
}

type TransactionContextInfo struct {
	*TransactionInfo
	BlockHash     string `json:"blockhash"`
	Confirmations uint32 `json:"confirmations"`
	Time          uint32 `json:"time"`
	BlockTime     uint32 `json:"blocktime"`
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

type InactiveArbitratorsInfo struct {
	Sponsor     string   `json:"sponsor"`
	Arbitrators []string `json:"arbitrators"`
	BlockHeight uint32   `json:"blockheight"`
}

type ActivateProducerInfo struct {
	NodePublicKey string `json:"nodepublickey"`
	Signature     string `json:"signature"`
}

type UpdateVersionInfo struct {
	StartHeight uint32 `json:"startheight"`
	EndHeight   uint32 `json:"endheight"`
}

type CRInfo struct {
	Code      string `json:"code"`
	CID       string `json:"cid"`
	DID       string `json:"did"`
	NickName  string `json:"nickname"`
	Url       string `json:"url"`
	Location  uint64 `json:"location"`
	Signature string `json:"signature"`
}

type UnregisterCRInfo struct {
	CID       string `json:"cid"`
	Signature string `json:"signature"`
}

type BudgetBaseInfo struct {
	Type   string `json:"type"`
	Stage  uint8  `json:"stage"`
	Amount string `json:"amount"`
}

type BudgetInfo struct {
	Type   string `json:"type"`
	Stage  uint8  `json:"stage"`
	Amount string `json:"amount"`
	Status string `json:"status"`
}

type CRCProposalInfo struct {
	ProposalType             string           `json:"proposaltype"`
	CategoryData             string           `json:"categorydata"`
	OwnerPublicKey           string           `json:"ownerpublickey"`
	DraftHash                string           `json:"drafthash"`
	Budgets                  []BudgetBaseInfo `json:"budgets"`
	Recipient                string           `json:"recipient"`
	Signature                string           `json:"signature"`
	CRCouncilMemberDID       string           `json:"crcouncilmemberdid"`
	CRCouncilMemberSignature string           `json:"crcouncilmembersignature"`
	Hash                     string           `json:"hash"`
}

type CRCProposalReviewInfo struct {
	ProposalHash string `json:"proposalhash"`
	VoteResult   string `json:"voteresult"`
	OpinionHash  string `json:"opinionhash"`
	DID          string `json:"did"`
	Sign         string `json:"sign"`
}

type CRCProposalTrackingInfo struct {
	ProposalTrackingType        string `json:"proposaltrackingtype"`
	ProposalHash                string `json:"proposalhash"`
	MessageHash                 string `json:"messagehash"`
	Stage                       uint8  `json:"stage"`
	OwnerPublicKey              string `json:"ownerpublickey"`
	NewOwnerPublicKey           string `json:"newownerpublickey"`
	OwnerSignature              string `json:"ownersignature"`
	NewOwnerSignature           string `json:"newownersignature"`
	SecretaryGeneralOpinionHash string `json:"secretarygeneralopinionhash"`
	SecretaryGeneralSignature   string `json:"secretarygeneralsignature"`
}

type CRCProposalWithdrawInfo struct {
	ProposalHash   string `json:"proposalhash"`
	OwnerPublicKey string `json:"ownerpublickey"`
	Signature      string `json:"signature"`
}

type UTXOInfo struct {
	TxType        byte   `json:"txtype"`
	TxID          string `json:"txid"`
	AssetID       string `json:"assetid"`
	VOut          uint16 `json:"vout"`
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
