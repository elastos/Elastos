// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package p2p

import (
	"bytes"
	"fmt"
	"io"
	"net"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

const (
	CMDSize         = 12
	CMDOffset       = 4
	ChecksumSize    = 4
	HeaderSize      = 24
	BlocksCacheSize = 2

	// MaxMessagePayload is the maximum bytes a message can be regardless of other
	// individual limits imposed by messages themselves.
	MaxMessagePayload = 1024 * 1024 * 32 // 32MB

	// WriteMessageTimeOut is the max time of write message, then the peer will
	// disconnect.
	WriteMessageTimeOut = 10 * time.Minute

	// WriteMessageTimeOut is the max time of read message, then the peer will
	// disconnect.
	ReadMessageTimeOut = 10 * time.Minute
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
	CmdDAddr       = "daddr"
)

var (
	mtx                sync.Mutex
	blockHashesCache   []common.Uint256
	blockConfirmsCache []bool
	blocksCache        map[common.Uint256]map[bool][]byte

	ErrInvalidHeader   = fmt.Errorf("invalid message header")
	ErrInvalidPayload  = fmt.Errorf("invalid message payload")
	ErrUnmatchedMagic  = fmt.Errorf("unmatched magic")
	ErrMsgSizeExceeded = fmt.Errorf("message size exceeded")
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
func ReadMessage(r net.Conn, magic uint32, timeout time.Duration,
	makeEmptyMessage MakeEmptyMessage) (Message, error) {
	// Set read deadline
	err := r.SetReadDeadline(time.Now().Add(timeout))
	if err != nil {
		return nil, fmt.Errorf("set read deadline failed %s", err.Error())
	}

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
func WriteMessage(w net.Conn, magic uint32, msg Message, timeout time.Duration,
	getDposBlock func(msg Message) (*types.DposBlock, bool)) error {
	// Serialize message
	var payload []byte
	mtx.Lock()
	dposBlock, ok := getDposBlock(msg)
	if ok {
		hash := dposBlock.Hash()
		cacheBlock, exist := blocksCache[hash]
		if exist {
			if block, _ := cacheBlock[dposBlock.HaveConfirm]; block != nil {
				payload = block
			}
		}
		if !exist {
			buf := new(bytes.Buffer)
			if err := dposBlock.Serialize(buf); err != nil {
				mtx.Unlock()
				return fmt.Errorf("write message serialize message failed %s", err.Error())
			}
			if len(blockHashesCache) >= BlocksCacheSize {
				delete(blocksCache[blockHashesCache[0]], blockConfirmsCache[0])
				blockHashesCache = blockHashesCache[1:BlocksCacheSize]
				blockConfirmsCache = blockConfirmsCache[1:BlocksCacheSize]
			}

			if blocksCache[hash] != nil {
				blocksCache[hash][dposBlock.HaveConfirm] = buf.Bytes()
			} else {
				cache := make(map[bool][]byte)
				cache[dposBlock.HaveConfirm] = buf.Bytes()
				blocksCache[hash] = cache
			}
			blockHashesCache = append(blockHashesCache, hash)
			blockConfirmsCache = append(blockConfirmsCache, dposBlock.HaveConfirm)
			payload = buf.Bytes()
		}
	}
	mtx.Unlock()
	if payload == nil {
		buf := new(bytes.Buffer)
		if err := msg.Serialize(buf); err != nil {
			return fmt.Errorf("serialize message failed %s", err.Error())
		}
		payload = buf.Bytes()
	}

	// Enforce maximum overall message payload.
	if len(payload) > MaxMessagePayload {
		return ErrMsgSizeExceeded
	}

	// Create message header
	hdr, err := BuildHeader(magic, msg.CMD(), payload).Serialize()
	if err != nil {
		return fmt.Errorf("serialize message header failed %s", err.Error())
	}

	// Set write deadline
	err = w.SetWriteDeadline(time.Now().Add(timeout))
	if err != nil {
		return fmt.Errorf("set write deadline failed %s", err.Error())
	}

	// Write header
	if _, err = w.Write(hdr); err != nil {
		return err
	}

	// Write payload
	_, err = w.Write(payload)
	return err
}

func init() {
	blockHashesCache = make([]common.Uint256, 0, BlocksCacheSize)
	blockConfirmsCache = make([]bool, 0, BlocksCacheSize)
	blocksCache = make(map[common.Uint256]map[bool][]byte)
}
