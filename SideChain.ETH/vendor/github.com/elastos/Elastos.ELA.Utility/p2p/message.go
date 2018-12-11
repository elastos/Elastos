package p2p

import (
	"bytes"
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	CMDSize      = 12
	CMDOffset    = 4
	ChecksumSize = 4
	HeaderSize   = 24

	// MaxMessagePayload is the maximum bytes a message can be regardless of other
	// individual limits imposed by messages themselves.
	MaxMessagePayload = 1024 * 1024 * 32 // 32MB
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
	CmdTxFilter    = "txfilter"
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

// MakeEmptyMessage is a function to make message by the given command.
type MakeEmptyMessage func(command string) (Message, error)

// ReadMessage reads, validates, and parse the Message from r for the
// provided magic.
func ReadMessage(r io.Reader, magic uint32, makeEmptyMessage MakeEmptyMessage) (Message, error) {
	// Read message header
	var headerBytes [HeaderSize]byte
	if _, err := io.ReadFull(r, headerBytes[:]); err != nil {
		return nil, err
	}

	// Deserialize message header
	var hdr Header
	if err := hdr.Deserialize(headerBytes[:]); err != nil {
		return nil, ErrInvalidHeader
	}

	// Check for messages from wrong network
	if hdr.Magic != magic {
		return nil, ErrUnmatchedMagic
	}

	// Create struct of appropriate message type based on the command.
	msg, err := makeEmptyMessage(hdr.GetCMD())
	if err != nil {
		return nil, err
	}

	// Check for message length
	if hdr.Length > msg.MaxLength() {
		return nil, ErrMsgSizeExceeded
	}

	// Read payload
	payload := make([]byte, hdr.Length)
	_, err = io.ReadFull(r, payload[:])
	if err != nil {
		return nil, err
	}

	// Verify checksum
	if err := hdr.Verify(payload); err != nil {
		return nil, ErrInvalidPayload
	}

	// Deserialize message
	if err := msg.Deserialize(bytes.NewBuffer(payload)); err != nil {
		return nil, fmt.Errorf("deserialize message %s failed %s", msg.CMD(), err.Error())
	}

	return msg, nil
}

// WriteMessage writes a Message to w including the necessary header
// information.
func WriteMessage(w io.Writer, magic uint32, msg Message) error {
	// Serialize message
	buf := new(bytes.Buffer)
	if err := msg.Serialize(buf); err != nil {
		return fmt.Errorf("serialize message failed %s", err.Error())
	}
	payload := buf.Bytes()

	// Enforce maximum overall message payload.
	if len(payload) > MaxMessagePayload {
		return ErrMsgSizeExceeded
	}

	// Create message header
	hdr, err := BuildHeader(magic, msg.CMD(), payload).Serialize()
	if err != nil {
		return fmt.Errorf("serialize message header failed %s", err.Error())
	}

	// Write header
	if _, err = w.Write(hdr); err != nil {
		return err
	}

	// Write payload
	_, err = w.Write(payload)
	return err
}
