package message

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"

	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	. "Elastos.ELA/net/protocol"
	"SPVWallet/core"
)

type Messager interface {
	Verify([]byte) error
	Serialization() ([]byte, error)
	Deserialization([]byte) error
	Handle(Noder) error
}

func BuildMessage(cmd string, body []byte) ([]byte, error) {
	hdr, err := BuildHeader(cmd, body).Serialization()
	if err != nil {
		return nil, err
	}

	return append(hdr, body...), nil
}

// The network communication message header
type Header struct {
	Magic uint32
	//ID	 uint64
	CMD      [MSGCMDLEN]byte // The message type
	Length   uint32
	Checksum [CHECKSUMLEN]byte
}

func NewHeader(cmd string, checksum []byte, length int) *Header {
	header := new(Header)
	// Write Magic
	header.Magic = config.Parameters.Magic
	// Write CMD
	copy(header.CMD[:len(cmd)], cmd)
	// Write length
	header.Length = uint32(length)
	// Write checksum
	copy(header.Checksum[:], checksum[:CHECKSUMLEN])

	return header
}

func BuildHeader(cmd string, msg []byte) *Header {
	// Calculate checksum
	checksum := core.Sha256D(msg)
	return NewHeader(cmd, checksum[:], len(msg))
}

// Alloc different message stucture
// @t the message name or type
// @len the message length only valid for varible length structure
//
// Return:
// @messager the messager interface
// @error  error code
// FixMe fix the ugly multiple return.
func AllocMsg(t string, length int) Messager {
	switch t {
	case "version":
		var msg version
		// TODO fill the header and type
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "verack":
		var msg verACK
		copy(msg.Header.CMD[0:len(t)], t)
		return &msg
	case "getaddr":
		var msg addrReq
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "addr":
		var msg addr
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "filterload":
		var msg FilterLoad
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "inv":
		var msg Inv
		copy(msg.CMD[0:len(t)], t)
		// the 1 is the inv type lenght
		msg.P.Blk = make([]byte, length-MSGHDRLEN)
		return &msg
	case "getdata":
		var msg dataReq
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "block":
		var msg block
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "tx":
		var msg trn
		copy(msg.CMD[0:len(t)], t)
		//if (message.Payload.Length <= 1024 * 1024)
		//OnInventoryReceived(Transaction.DeserializeFrom(message.Payload));
		return &msg
	case "getblocks":
		var msg blocksReq
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "notfound":
		var msg notFound
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "ping":
		var msg ping
		copy(msg.CMD[0:len(t)], t)
		return &msg
	case "pong":
		var msg pong
		copy(msg.CMD[0:len(t)], t)
		return &msg
	default:
		log.Warn("Unknown message type")
		return nil
	}
}

func MsgType(buf []byte) (string, error) {
	cmd := buf[CMDOFFSET: CMDOFFSET+MSGCMDLEN]
	n := bytes.IndexByte(cmd, 0)
	if n < 0 || n >= MSGCMDLEN {
		return "", errors.New("Unexpected length of CMD command")
	}
	s := string(cmd[:n])
	return s, nil
}

// TODO combine all of message alloc in one function via interface
func NewMsg(t string, n Noder) ([]byte, error) {
	switch t {
	case "version":
		return NewVersion(n)
	case "verack":
		return NewVerack()
	case "getaddr":
		return newGetAddr()

	default:
		return nil, errors.New("Unknown message type")
	}
}

// FIXME the length exceed int32 case?
func HandleNodeMsg(node Noder, buf []byte, len int) error {
	if len < MSGHDRLEN {
		log.Warn("Unexpected size of received message")
		return errors.New("Unexpected size of received message")
	}

	log.Debugf("Received data len:  %d\n%x", len, buf[:len])

	s, err := MsgType(buf)
	if err != nil {
		log.Error("Message type parsing error")
		return err
	}

	if s == "inv" || s == "block" {
		node.LocalNode().AcqSyncBlkReqSem()
		msg := AllocMsg(s, len)
		if msg == nil {
			log.Error(fmt.Sprintf("Allocation message %s failed", s))
			return errors.New("Allocation message failed")
		}
		// Todo attach a node pointer to each message
		// Todo drop the message when verify/deseria packet error
		msg.Deserialization(buf[:len])
		msg.Verify(buf[MSGHDRLEN:len])

		errr := msg.Handle(node)
		node.LocalNode().RelSyncBlkReqSem()
		return errr
	} else {
		msg := AllocMsg(s, len)
		if msg == nil {
			log.Error(fmt.Sprintf("Allocation message %s failed", s))
			return errors.New("Allocation message failed")
		}
		// Todo attach a node pointer to each message
		// Todo drop the message when verify/deseria packet error
		msg.Deserialization(buf[:len])
		msg.Verify(buf[MSGHDRLEN:len])

		errr := msg.Handle(node)
		return errr
	}
}

func ValidMsgHdr(buf []byte) bool {
	var h Header
	h.Deserialization(buf)
	//TODO: verify hdr checksum
	if h.Magic != config.Parameters.Magic {
		return false
	}
	return true
}

func PayloadLen(buf []byte) int {
	var h Header
	h.Deserialization(buf)
	return int(h.Length)
}

func (hdr *Header) init(cmd string, checksum []byte, length uint32) {
	hdr.Magic = config.Parameters.Magic
	copy(hdr.CMD[0:uint32(len(cmd))], cmd)
	copy(hdr.Checksum[:], checksum[:CHECKSUMLEN])
	hdr.Length = length
}

// Verify the message header information
// @p payload of the message
func (hdr Header) Verify(buf []byte) error {
	if hdr.Magic != config.Parameters.Magic {
		log.Error(fmt.Sprintf("Unmatched magic number 0x%0x", hdr.Magic))
		return errors.New("Unmatched magic number")
	}
	t := sha256.Sum256(buf)
	s := sha256.Sum256(t[:])
	// Currently we only need the front 4 bytes as checksum
	checkSum := s[:CHECKSUMLEN]

	if bytes.Equal(hdr.Checksum[:], checkSum[:]) == false {
		str1 := hex.EncodeToString(hdr.Checksum[:])
		str2 := hex.EncodeToString(checkSum[:])
		log.Warn(fmt.Sprintf("Message Checksum error, Received checksum %s Wanted checksum: %s",
			str1, str2))
		return errors.New("Message Checksum error")
	}

	return nil
}

func (msg *Header) Deserialization(p []byte) error {

	buf := bytes.NewBuffer(p[0:MSGHDRLEN])
	err := binary.Read(buf, binary.LittleEndian, msg)
	return err
}

// FIXME how to avoid duplicate serial/deserial function as
// most of them are the same
func (hdr Header) Serialization() ([]byte, error) {
	var buf bytes.Buffer
	err := binary.Write(&buf, binary.LittleEndian, hdr)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), err
}

func (hdr Header) Handle(n Noder) error {
	return nil
}
