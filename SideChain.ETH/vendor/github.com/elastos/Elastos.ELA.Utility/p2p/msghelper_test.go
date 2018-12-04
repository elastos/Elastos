package p2p

import (
	"crypto/rand"
	"errors"
	"fmt"
	"io"
	"net"
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

type TestMsg struct {
	cmd  string
	len  int
	body []byte
}

func NewTestMsg(cmd string, body []byte) *TestMsg {
	return &TestMsg{cmd: cmd, len: len(body), body: body}
}

func (m *TestMsg) CMD() string {
	return m.cmd
}

func (m *TestMsg) MaxLength() uint32 {
	switch m.cmd {
	case CmdBlock:
		return 1024 * 1024 * 8
	default:
		return 1024 * 1024
	}
}

func (m *TestMsg) Serialize(writer io.Writer) error {
	_, err := writer.Write(m.body)
	return err
}

func (m *TestMsg) Deserialize(reader io.Reader) error {
	m.body = make([]byte, m.len)
	_, err := reader.Read(m.body)
	return err
}

type TestMsgHandler struct {
	magic, maxMsgSize, port uint32
	thisHelper              *MsgHelper
	thatHelper              *MsgHelper
	errChan                 chan error
	msgChan                 chan Message
}

func NewTestMsgHandler(magic, maxMsgSize, port uint32) *TestMsgHandler {
	return &TestMsgHandler{
		magic:      magic,
		maxMsgSize: maxMsgSize,
		port:       port,
		errChan:    make(chan error, 1024),
		msgChan:    make(chan Message),
	}
}

func (h *TestMsgHandler) Start() error {
	go h.listen()

	conn, err := net.Dial("tcp", fmt.Sprint("127.0.0.1:", h.port))
	if err != nil {
		return err
	}
	h.thisHelper = NewMsgHelper(h.magic, h.maxMsgSize, conn, h)
	h.thisHelper.Read()
	return nil
}

func (h *TestMsgHandler) WriteMsg(msg Message) (Message, error) {
	h.thisHelper.Write(msg)
	select {
	case msg := <-h.msgChan:
		return msg, nil
	case err := <-h.errChan:
		return nil, err
	}
}

func (h *TestMsgHandler) Write(msg []byte) (Message, error) {
	h.thisHelper.conn.Write(msg)
	select {
	case msg := <-h.msgChan:
		return msg, nil
	case err := <-h.errChan:
		return nil, err
	}
}

func (h *TestMsgHandler) ReadMsg(msg Message) (Message, error) {
	h.thatHelper.Write(msg)
	select {
	case msg := <-h.msgChan:
		return msg, nil
	case err := <-h.errChan:
		return nil, err
	}
}

func (h *TestMsgHandler) Read(msg []byte) (Message, error) {
	h.thatHelper.conn.Write(msg)
	select {
	case msg := <-h.msgChan:
		return msg, nil
	case err := <-h.errChan:
		return nil, err
	}
}

func (h *TestMsgHandler) Clear() {
clear:
	for {
		select {
		case <-h.msgChan:
		case <-h.errChan:
		default:
			break clear
		}
	}
}

func (h *TestMsgHandler) Stop() {
	h.thisHelper.conn.Close()
	h.thatHelper.conn.Close()
}

func (h *TestMsgHandler) listen() {
	ln, err := net.Listen("tcp", fmt.Sprint(":", h.port))
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}
	for {
		conn, err := ln.Accept()
		if err != nil {
			break
		}
		h.thatHelper = NewMsgHelper(h.magic, h.maxMsgSize, conn, h)
		h.thatHelper.Read()
	}
}

// When something wrong on read or decode message
// this method will callback the error
func (h *TestMsgHandler) OnError(err error) {
	h.errChan <- err
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *TestMsgHandler) OnMakeMessage(cmd string) (message Message, err error) {
	switch cmd {
	case
		CmdVersion,
		CmdVerAck,
		CmdGetAddr,
		CmdAddr,
		CmdPing,
		CmdPong,
		CmdFilterLoad,
		CmdGetBlocks,
		CmdInv,
		CmdGetData,
		CmdBlock,
		CmdTx,
		CmdNotFound,
		CmdMemPool,
		CmdReject:
		message = NewTestMsg(cmd, nil)
	default:
		err = errors.New("unknown message type")
	}

	return message, err
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *TestMsgHandler) OnMessageDecoded(msg Message) {
	h.msgChan <- msg
}

func TestHeader_Read_And_Write(t *testing.T) {
	handler := NewTestMsgHandler(987654321, 1024*1024*8, 22222)

	err := handler.Start()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	// no cmd
	var body = make([]byte, 1024*8)
	rand.Read(body)
	header, err := BuildHeader(987654321, "", body).Serialize()
	assert.NoError(t, err)
	_, err = handler.Read(append(header, body...))
	assert.EqualError(t, err, "[P2P] make message failed unknown message type")
	_, err = handler.Write(append(header, body...))
	assert.EqualError(t, err, "[P2P] make message failed unknown message type")

	// unknown cmd
	header, err = BuildHeader(987654321, "unknown", body).Serialize()
	assert.NoError(t, err)
	_, err = handler.Read(append(header, body...))
	assert.EqualError(t, err, "[P2P] make message failed unknown message type")
	_, err = handler.Write(append(header, body...))
	assert.EqualError(t, err, "[P2P] make message failed unknown message type")

	// cmd too long
	hdr := BuildHeader(987654321, "", body)
	for i := range hdr.CMD {
		hdr.CMD[i] = 1
	}
	header, err = hdr.Serialize()
	assert.NoError(t, err)
	_, err = handler.Read(append(header, body...))
	assert.EqualError(t, err, "[P2P] invalid message header")
	_, err = handler.Write(append(header, body...))
	assert.EqualError(t, err, "[P2P] invalid message header")

	// tamper message body
	hdr = BuildHeader(987654321, CmdVersion, body)
	header, err = hdr.Serialize()
	assert.NoError(t, err)
	rand.Read(body)
	_, err = handler.Read(append(header, body...))
	assert.EqualError(t, err, "[P2P] verify message header failed unmatched body checksum")
	_, err = handler.Write(append(header, body...))
	assert.EqualError(t, err, "[P2P] verify message header failed unmatched body checksum")

	// disconnect
	handler.Stop()
}

func TestMessages_Read_And_Write(t *testing.T) {
	handler := NewTestMsgHandler(987654321, 1024*1024*8, 22223)

	err := handler.Start()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// cmds
	cmds := []string{
		CmdVersion,
		CmdVerAck,
		CmdGetAddr,
		CmdAddr,
		CmdPing,
		CmdPong,
		CmdFilterLoad,
		CmdGetBlocks,
		CmdInv,
		CmdGetData,
		CmdBlock,
		CmdTx,
		CmdNotFound,
		CmdMemPool,
		CmdReject,
	}
	for _, cmd := range cmds {
		var body = make([]byte, 1024*1024)
		rand.Read(body)
		msg, err := handler.WriteMsg(NewTestMsg(cmd, body))
		if assert.NoError(t, err) {
			assert.Equal(t, cmd, msg.CMD())
		}
		msg, err = handler.ReadMsg(NewTestMsg(cmd, body))
		if assert.NoError(t, err) {
			assert.Equal(t, cmd, msg.CMD())
		}
	}

	// 8MB big message
	body := make([]byte, 1024*1024*8)
	msg, err := handler.WriteMsg(NewTestMsg(CmdBlock, body))
	if assert.NoError(t, err) {
		assert.Equal(t, CmdBlock, msg.CMD())
	}
	msg, err = handler.ReadMsg(NewTestMsg(CmdBlock, body))
	if assert.NoError(t, err) {
		assert.Equal(t, CmdBlock, msg.CMD())
	}

	// disconnect
	handler.Stop()
}


func TestMsgSizeExceeded(t *testing.T) {
	handler := NewTestMsgHandler(987654321, 1024*1024*6, 22224)

	err := handler.Start()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// exceeded message size
	body := make([]byte, 1024*1024*6+1)
	_, err = handler.WriteMsg(NewTestMsg(CmdBlock, body))
	assert.EqualError(t, err, "[P2P] message size exceeded")
	handler.Clear()

	// disconnect
	handler.Stop()
}

func TestMaxLengthExceeded(t *testing.T) {
	handler := NewTestMsgHandler(987654321, 1024*1024*8, 22225)

	err := handler.Start()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// exceeded message size
	body := make([]byte, 1024*1024+1)
	_, err = handler.WriteMsg(NewTestMsg(CmdFilterLoad, body))
	assert.EqualError(t, err, "[P2P] message size exceeded")

	// disconnect
	handler.Stop()
}

