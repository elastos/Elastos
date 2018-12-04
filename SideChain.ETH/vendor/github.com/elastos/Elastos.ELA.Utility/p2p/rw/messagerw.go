package rw

import (
	"io"
	"fmt"
	"bytes"

	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg/v0"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type MessageConfig struct {
	ProtocolVersion uint32 // The P2P network protocol version
	MakeTx          func() *msg.Tx
	MakeBlock       func() *msg.Block
	MakeMerkleBlock func() *msg.MerkleBlock
}

type MessageRW interface {
	SetConfig(config MessageConfig)
	ReadMessage(r io.Reader) (p2p.Message, error)
	WriteMessage(w io.Writer, msg p2p.Message) error
}

func GetMesssageRW(magic uint32) MessageRW {
	return &rw{magic: magic}
}

type rw struct {
	magic  uint32
	config MessageConfig
}

func (rw *rw) SetConfig(config MessageConfig) {
	rw.config = config
}

func (rw *rw) ReadMessage(r io.Reader) (p2p.Message, error) {
	// Read message header
	var headerBytes [p2p.HeaderSize]byte
	if _, err := io.ReadFull(r, headerBytes[:]); err != nil {
		return nil, err
	}

	// Deserialize message header
	var hdr p2p.Header
	if err := hdr.Deserialize(headerBytes[:]); err != nil {
		return nil, p2p.ErrInvalidHeader
	}

	// Check for messages from wrong network
	if hdr.Magic != rw.magic {
		return nil, p2p.ErrUnmatchedMagic
	}

	// Create struct of appropriate message type based on the command.
	msg, err := rw.makeEmptyMessage(hdr.GetCMD())
	if err != nil {
		return nil, err
	}

	// Check for message length
	if hdr.Length > msg.MaxLength() {
		return nil, p2p.ErrMsgSizeExceeded
	}

	// Read payload
	payload := make([]byte, hdr.Length)
	_, err = io.ReadFull(r, payload[:])
	if err != nil {
		return nil, err
	}

	// Verify checksum
	if err := hdr.Verify(payload); err != nil {
		return nil, p2p.ErrInvalidPayload
	}

	// Deserialize message
	if err := msg.Deserialize(bytes.NewBuffer(payload)); err != nil {
		return nil, fmt.Errorf("deserialize message %s failed %s", msg.CMD(), err.Error())
	}

	return msg, nil
}

func (rw *rw) WriteMessage(w io.Writer, msg p2p.Message) error {
	// Serialize message
	buf := new(bytes.Buffer)
	if err := msg.Serialize(buf); err != nil {
		return fmt.Errorf("serialize message failed %s", err.Error())
	}
	payload := buf.Bytes()

	// Enforce maximum overall message payload.
	if len(payload) > p2p.MaxMessagePayload {
		return p2p.ErrMsgSizeExceeded
	}

	// Create message header
	hdr, err := p2p.BuildHeader(rw.magic, msg.CMD(), payload).Serialize()
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

func (rw *rw) makeEmptyMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case p2p.CmdVersion:
		message = new(msg.Version)

	case p2p.CmdVerAck:
		message = new(msg.VerAck)

	case p2p.CmdGetAddr:
		message = new(msg.GetAddr)

	case p2p.CmdAddr:
		message = new(msg.Addr)

	case p2p.CmdGetBlocks:
		message = new(msg.GetBlocks)

	case p2p.CmdBlock:
		message = rw.config.MakeBlock()

	case p2p.CmdInv:
		switch rw.config.ProtocolVersion {
		case p2p.EIP001Version:
			message = new(msg.Inventory)
		default:
			message = new(v0.Inv)
		}

	case p2p.CmdGetData:
		switch rw.config.ProtocolVersion {
		case p2p.EIP001Version:
			message = new(msg.GetData)
		default:
			message = new(v0.GetData)
		}

	case p2p.CmdNotFound:
		switch rw.config.ProtocolVersion {
		case p2p.EIP001Version:
			message = new(msg.NotFound)
		default:
			message = new(v0.NotFound)
		}

	case p2p.CmdTx:
		message = rw.config.MakeTx()

	case p2p.CmdPing:
		message = new(msg.Ping)

	case p2p.CmdPong:
		message = new(msg.Pong)

	case p2p.CmdMemPool:
		message = new(msg.MemPool)

	case p2p.CmdFilterAdd:
		message = new(msg.FilterAdd)

	case p2p.CmdFilterClear:
		message = new(msg.FilterClear)

	case p2p.CmdFilterLoad:
		message = new(msg.FilterLoad)

	case p2p.CmdMerkleBlock:
		message = rw.config.MakeMerkleBlock()

	case p2p.CmdReject:
		message = new(msg.Reject)

	default:
		return nil, fmt.Errorf("unhandled command [%s]", cmd)
	}
	return message, nil
}
