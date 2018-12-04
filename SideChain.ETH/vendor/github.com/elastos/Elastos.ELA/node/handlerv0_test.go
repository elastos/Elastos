package node

import (
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg/v0"
	"github.com/stretchr/testify/assert"
)

type TestHandlerV0 struct {
	TestHandlerBase
}

func newTestHandlerV0(t *testing.T, port uint16) *TestHandlerV0 {
	this, that := newTestNode(t, port)
	thisHandler := newTestMsgHandler(NewHandlerV0(this))
	thatHandler := newTestMsgHandler(NewHandlerV0(that))

	this.UpdateMsgHelper(thisHandler)
	that.UpdateMsgHelper(thatHandler)

	handler := &TestHandlerV0{
		TestHandlerBase: TestHandlerBase{
			this:        this,
			that:        that,
			thisHandler: thisHandler,
			thatHandler: thatHandler,
		},
	}
	return handler
}

func TestNewHandlerV0(t *testing.T) {
	initLocalNode(t)

	handler := newTestHandlerV0(t, config.Parameters.NodePort)

	// ping message
	response, err := handler.Send(newMessage(p2p.CmdPing))
	assert.NoError(t, err)
	assert.Equal(t, p2p.CmdPong, response.CMD())

	// pong message
	err = handler.Write(newMessage(p2p.CmdPong))
	assert.NoError(t, err)

	// message getblocks
	err = handler.Write(newMessage(p2p.CmdGetBlocks))
	assert.NoError(t, err)
	time.Sleep(time.Millisecond * 100)
	invs := make([]*v0.Inv, 0)
	for len(handler.thisHandler.msgChan) > 0 {
		response, err := handler.Read()
		assert.NoError(t, err)
		assert.Equal(t, p2p.CmdInv, response.CMD())
		if len(response.(*v0.Inv).Hashes) > p2p.MaxHeaderHashes {
			t.Errorf("Inventory hashes overload length %d", len(response.(*v0.Inv).Hashes))
		}
		invs = append(invs, response.(*v0.Inv))
	}
	for len(handler.thatHandler.msgChan) > 0 {
		response, err := handler.Pop()
		assert.NoError(t, err)
		assert.Equal(t, p2p.CmdGetBlocks, response.CMD())
	}

	// message inv
	err = handler.Write(invs[0])
	assert.NoError(t, err)
	time.Sleep(time.Millisecond * 100)
	for len(handler.thisHandler.msgChan) > 0 {
		response, err := handler.Read()
		assert.NoError(t, err)
		assert.Equal(t, p2p.CmdGetData, response.CMD())
	}

	// message getdata
	blocks := make([]p2p.Message, 0, len(invs[0].Hashes))
	handler.that.(*node).id = handler.this.ID()
	LocalNode.AddNeighborNode(handler.that)
	for _, hash := range invs[0].Hashes {
		err = handler.Write(v0.NewGetData(*hash))
		assert.NoError(t, err)
		response, _ := handler.Read()
		switch response.(type) {
		case *msg.Ping:
			handler.Pop()
			response, err = handler.Read()
		}
		assert.Equal(t, p2p.CmdBlock, response.CMD())
		blocks = append(blocks, response)
	}

	// message getdata notfound
	err = handler.Write(v0.NewGetData(common.EmptyHash))
	assert.EqualError(t, err, "leveldb: not found")
	response, err = handler.Read()
	assert.NoError(t, err)
	assert.Equal(t, p2p.CmdNotFound, response.CMD())

	// message block
	LocalNode.SetSyncHeaders(false)
	for _, block := range blocks {
		err = handler.Write(block)
		assert.EqualError(t, err, "received duplicated block")
	}
	LocalNode.DelNeighborNode(handler.that.ID())

	// message tx
	err = handler.Write(newMessage(p2p.CmdTx))
	assert.EqualError(t, err, "[HandlerBase] VerifyTransaction failed when AppendToTxnPool")
}
