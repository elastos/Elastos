package p2p

import (
	"bytes"
	"fmt"
	"net"
)

const MaxBufLen = 1024 * 16

// The interface to callback message read errors, message creation and decoded message.
type MsgHandler interface {
	// When something wrong on read or decode message
	// this method will callback the error
	OnError(err error)

	// After message header decoded, this method will be
	// called to create the message instance with the CMD
	// which is the message type of the received message
	OnMakeMessage(cmd string) (Message, error)

	// After message has been successful decoded, this method
	// will be called to pass the decoded message instance
	OnMessageDecoded(msg Message)
}

type MsgHelper struct {
	buf        []byte
	len        int
	magic      uint32
	maxMsgSize uint32
	conn       net.Conn
	handler    MsgHandler
}

// NewMsgHelper create a new instance of *MsgHelper
func NewMsgHelper(magic, maxMsgSize uint32, conn net.Conn, handler MsgHandler) *MsgHelper {
	helper := new(MsgHelper)
	helper.magic = magic
	helper.maxMsgSize = maxMsgSize
	helper.conn = conn
	helper.handler = handler
	return helper
}

func (helper *MsgHelper) Update(handler MsgHandler) {
	helper.handler = handler
}

func (helper *MsgHelper) Read() {
	go func() {
		var buf = make([]byte, MaxBufLen)
		for {
			len, err := helper.conn.Read(buf[0 : MaxBufLen-1])
			buf[MaxBufLen-1] = 0 //Prevent overflow
			switch err {
			case nil:
				helper.unpack(buf[:len])
			default:
				goto ERROR
			}
		}
	ERROR:
		helper.handler.OnError(ErrDisconnected)
	}()
}

func (helper *MsgHelper) Write(msg Message) {
	buf := new(bytes.Buffer)
	err := msg.Serialize(buf)
	if err != nil {
		helper.handler.OnError(fmt.Errorf("[P2P] serialize message failed %s", err.Error()))
		return
	}
	hdr, err := BuildHeader(helper.magic, msg.CMD(), buf.Bytes()).Serialize()
	if err != nil {
		helper.handler.OnError(fmt.Errorf("[P2P] serialize message header failed %s", err.Error()))
		return
	}

	_, err = helper.conn.Write(append(hdr, buf.Bytes()...))
	if err != nil {
		helper.handler.OnError(ErrDisconnected)
	}
}

func (helper *MsgHelper) append(msg []byte) []byte {
	helper.buf = append(helper.buf, msg...)
	return helper.buf
}

func (helper *MsgHelper) reset() {
	helper.buf = nil
	helper.len = 0
}

func (helper *MsgHelper) unpack(buf []byte) {
	if len(buf) == 0 {
		return
	}

	// Buffer message header
	if helper.len == 0 {
		index := HeaderSize - len(helper.buf)
		if index > len(buf) { // header not finished, continue read
			index = len(buf)
			helper.append(buf[:index])
			return
		}

		var header Header
		if err := header.Deserialize(helper.append(buf[:index])); err != nil {
			helper.handler.OnError(ErrInvalidHeader)
			helper.reset()
			return
		}

		if header.Magic != helper.magic {
			helper.handler.OnError(ErrUnmatchedMagic)
			helper.reset()
			return
		}

		if header.Length > helper.maxMsgSize {
			helper.handler.OnError(ErrMsgSizeExceeded)
			helper.reset()
			return
		}

		helper.len = int(header.Length)
		buf = buf[index:]
	}
	msgLen := helper.len

	// Read part of the message
	if len(buf) < msgLen {
		helper.append(buf[:])
		helper.len = msgLen - len(buf)
		return
	}

	// decode received message
	helper.decode(helper.append(buf[:msgLen]))
	helper.reset()

	// unpack next message
	helper.unpack(buf[msgLen:])
}

func (helper *MsgHelper) decode(buf []byte) {
	if len(buf) < HeaderSize {
		helper.handler.OnError(fmt.Errorf("[P2P] message Length is not enough"))
		return
	}

	hdr, err := verify(buf)
	if err != nil {
		helper.handler.OnError(fmt.Errorf("[P2P] verify message header failed %s", err.Error()))
		return
	}

	msg, err := helper.handler.OnMakeMessage(hdr.GetCMD())
	if err != nil {
		helper.handler.OnError(fmt.Errorf("[P2P] make message failed %s", err.Error()))
		return
	}

	if hdr.Length > msg.MaxLength() {
		helper.handler.OnError(ErrMsgSizeExceeded)
		return
	}

	err = msg.Deserialize(bytes.NewReader(buf[HeaderSize:]))
	if err != nil {
		helper.handler.OnError(
			fmt.Errorf("[P2P] deserialize message %s failed %s", msg.CMD(), err.Error()))
		return
	}

	helper.handler.OnMessageDecoded(msg)
}

func verify(buf []byte) (*Header, error) {
	header := new(Header)
	err := header.Deserialize(buf)
	if err = header.Verify(buf[HeaderSize:]); err != nil {
		return nil, err
	}
	return header, nil
}
