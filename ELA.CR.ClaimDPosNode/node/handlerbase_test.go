package node

import (
	"bytes"
	"fmt"
	"testing"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/stretchr/testify/assert"
)

/*
A TestHandlerBase is an extension class of the HandlerBase for test purpose.
It act just the same as the origin HandlerBase, but will return error messages
When using Write() method.
*/

type ITestMsgHandler interface {
	p2p.MsgHandler
	HandleMessage(message p2p.Message) error
}

type TestMsgHandler struct {
	handler ITestMsgHandler
	errChan chan error
	msgChan chan p2p.Message
}

func newTestMsgHandler(handler ITestMsgHandler) *TestMsgHandler {
	return &TestMsgHandler{
		handler: handler,
		errChan: make(chan error, 100),
		msgChan: make(chan p2p.Message, 100),
	}
}

func (h *TestMsgHandler) OnError(err error) {
	h.handler.OnError(err)
	h.msgChan <- nil
	h.errChan <- err
}

func (h *TestMsgHandler) OnMakeMessage(cmd string) (p2p.Message, error) {
	return h.handler.OnMakeMessage(cmd)
}

func (h *TestMsgHandler) OnMessageDecoded(message p2p.Message) {
	h.msgChan <- message
	h.errChan <- h.handler.HandleMessage(message)
}

type TestHandlerBase struct {
	this, that               protocol.Noder
	thisHandler, thatHandler *TestMsgHandler
}

func newTestHandlerBase(t *testing.T, port uint16) *TestHandlerBase {
	this, that := newTestNode(t, port)
	thisHandler := newTestMsgHandler(NewHandlerBase(this))
	thatHandler := newTestMsgHandler(NewHandlerBase(that))

	this.UpdateMsgHelper(thisHandler)
	that.UpdateMsgHelper(thatHandler)

	handler := &TestHandlerBase{
		this:        this,
		that:        that,
		thisHandler: thisHandler,
		thatHandler: thatHandler,
	}
	return handler
}

func (h *TestHandlerBase) Write(message p2p.Message) error {
	h.this.(*node).Write(message)
	msg, err := h.Pop()
	if err != nil {
		return err
	}
	buf1, buf2 := new(bytes.Buffer), new(bytes.Buffer)
	message.Serialize(buf1)
	msg.Serialize(buf2)
	if !bytes.Equal(buf1.Bytes(), buf2.Bytes()) {
		return fmt.Errorf("received message not equal to origin message")
	}
	return nil
}

func (h *TestHandlerBase) Pop() (p2p.Message, error) {
	return <-h.thatHandler.msgChan, <-h.thatHandler.errChan
}

func (h *TestHandlerBase) Read() (p2p.Message, error) {
	return <-h.thisHandler.msgChan, <-h.thisHandler.errChan
}

func (h *TestHandlerBase) Send(message p2p.Message) (p2p.Message, error) {
	if err := h.Write(message); err != nil {
		return nil, err
	}
	return h.Read()
}

func TestNewHandlerBase(t *testing.T) {
	initLocalNode(t)

	handler := newTestHandlerBase(t, config.Parameters.NodePort)

	// version message
	err := handler.Write(newMessage(p2p.CmdVersion))
	assert.EqualError(t, err, "unknown status to receive version")

	// verack message
	err = handler.Write(newMessage(p2p.CmdVerAck))
	assert.EqualError(t, err, "unknown status to received verack")

	// getaddr message
	response, err := handler.Send(newMessage(p2p.CmdGetAddr))
	assert.NoError(t, err)
	assert.Equal(t, p2p.CmdAddr, response.CMD())

	// addr message
	err = handler.Write(newMessage(p2p.CmdAddr))
	assert.NoError(t, err)
}
