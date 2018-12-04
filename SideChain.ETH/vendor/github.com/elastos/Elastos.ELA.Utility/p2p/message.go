package p2p

import (
	"fmt"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	CMDSize      = 12
	CMDOffset    = 4
	ChecksumSize = 4
	HeaderSize   = 24

	// MaxMessagePayload is the maximum bytes a message can be regardless of other
	// individual limits imposed by messages themselves.
	MaxMessagePayload = (1024 * 1024 * 32) // 32MB
)

const (
	CmdVersion     = "version"
	CmdVerAck      = "verack"
	CmdGetAddr     = "getaddr"
	CmdAddr        = "addr"
	CmdGetBlocks   = "getblocks"
	CmdInv         = "inv"
	CmdGetData     = "getdata"
	CmdNotFound    = "notfound"
	CmdBlock       = "block"
	CmdTx          = "tx"
	CmdPing        = "ping"
	CmdPong        = "pong"
	CmdMemPool     = "mempool"
	CmdFilterAdd   = "filteradd"
	CmdFilterClear = "filterclear"
	CmdFilterLoad  = "filterload"
	CmdMerkleBlock = "merkleblock"
	CmdReject      = "reject"
)

var (
	ErrDisconnected    = fmt.Errorf("[P2P] peer disconnected")
	ErrInvalidHeader   = fmt.Errorf("[P2P] invalid message header")
	ErrInvalidPayload  = fmt.Errorf("[P2P] invalid message payload")
	ErrUnmatchedMagic  = fmt.Errorf("[P2P] unmatched magic")
	ErrMsgSizeExceeded = fmt.Errorf("[P2P] message size exceeded")
)

// The message flying in the peer to peer network
type Message interface {
	// Get the message CMD parameter which is the type of this message
	CMD() string
	// Get the max payload size of this message
	MaxLength() uint32
	// A message is a serializable instance
	common.Serializable
}
