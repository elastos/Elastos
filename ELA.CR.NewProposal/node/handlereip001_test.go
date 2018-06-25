package node

import (
	"crypto/rand"
	"fmt"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/stretchr/testify/assert"
)

type TestHandlerEIP001 struct {
	TestHandlerBase
}

func newTestHandlerEIP001(t *testing.T, port uint16) *TestHandlerEIP001 {
	this, that := newTestNode(t, port)
	thisHandler := newTestMsgHandler(NewHandlerEIP001(this))
	thatHandler := newTestMsgHandler(NewHandlerEIP001(that))

	this.UpdateMsgHelper(thisHandler)
	that.UpdateMsgHelper(thatHandler)

	handler := &TestHandlerEIP001{
		TestHandlerBase: TestHandlerBase{
			this:        this,
			that:        that,
			thisHandler: thisHandler,
			thatHandler: thatHandler,
		},
	}
	return handler
}

func TestNewHandlerEIP001(t *testing.T) {
	initLocalNode(t)

	handler := newTestHandlerEIP001(t, config.Parameters.NodePort)

	// ping message
	response, err := handler.Send(newMessage(p2p.CmdPing))
	assert.NoError(t, err)
	assert.Equal(t, p2p.CmdPong, response.CMD())

	// pong message
	err = handler.Write(newMessage(p2p.CmdPong))
	assert.NoError(t, err)

	// filterload message
	assert.Equal(t, false, handler.that.BloomFilter().IsLoaded())
	err = handler.Write(newMessage(p2p.CmdFilterLoad))
	assert.NoError(t, err)
	assert.Equal(t, true, handler.that.BloomFilter().IsLoaded())

	// message getblocks
	err = handler.Write(newMessage(p2p.CmdGetBlocks))
	assert.NoError(t, err)
	time.Sleep(time.Millisecond * 100)
	invs := make([]*msg.Inventory, 0)
	for len(handler.thisHandler.msgChan) > 0 {
		response, err := handler.Read()
		assert.NoError(t, err)
		assert.Equal(t, p2p.CmdInv, response.CMD())
		if len(response.(*msg.Inventory).InvList) > p2p.MaxBlocksPerMsg {
			t.Errorf("Inventory hashes overload length %d", len(response.(*msg.Inventory).InvList))
		}
		invs = append(invs, response.(*msg.Inventory))
	}
	for len(handler.thatHandler.msgChan) > 0 {
		_, err := handler.Pop()
		assert.NoError(t, err)
	}

	// message inventory type tx
	inv := msg.NewInventory()
	for i := 0; i < 50; i++ {
		var hash common.Uint256
		rand.Read(hash[:])
		inv.AddInvVect(msg.NewInvVect(msg.InvTypeTx, &hash))
	}
	err = handler.Write(inv)
	assert.NoError(t, err)
	for len(handler.thisHandler.msgChan) > 0 {
		response, err := handler.Read()
		assert.NoError(t, err)
		assert.Equal(t, p2p.CmdGetData, response.CMD())
		for _, inv := range response.(*msg.GetData).InvList {
			assert.Equal(t, msg.InvTypeTx, inv.Type)
		}
	}

	// message inventory type block
	err = handler.Write(invs[0])
	assert.NoError(t, err)
	time.Sleep(time.Millisecond * 100)
	for len(handler.thisHandler.msgChan) > 0 {
		response, err := handler.Read()
		assert.NoError(t, err)
		assert.Equal(t, p2p.CmdGetData, response.CMD())
		for _, inv := range response.(*msg.GetData).InvList {
			assert.Equal(t, msg.InvTypeBlock, inv.Type)
		}
	}

	// message getdata notfound
	getData := msg.NewGetData()
	getData.InvList = []*msg.InvVect{msg.NewInvVect(msg.InvTypeBlock, &common.EmptyHash)}
	response, err = handler.Send(getData)
	assert.NoError(t, err)
	assert.Equal(t, p2p.CmdNotFound, response.CMD())

	// message getdata
	blocks := make([]p2p.Message, 0, len(invs[0].InvList))
	handler.that.(*node).id = handler.this.ID()
	LocalNode.AddNeighborNode(handler.that)
	getData.InvList = invs[0].InvList
	err = handler.Write(getData)
	assert.NoError(t, err)
	for i := 0; i < len(getData.InvList); i++ {
		response, _ := handler.Read()
		switch response.(type) {
		case *msg.Ping:
			handler.Pop()
			response, err = handler.Read()
		}
		assert.Equal(t, p2p.CmdBlock, response.CMD())
		blocks = append(blocks, response)
	}
	fmt.Printf("invlist length %d received blocks %d\n", len(getData.InvList), len(blocks))

	// message block
	for _, block := range blocks {
		err = handler.Write(block)
		assert.EqualError(t, err, fmt.Sprintf("receive duplicated block %s",
			block.(*msg.Block).Block.(*core.Block).Hash().String()))
	}

	// message tx
	LocalNode.SetSyncHeaders(false)
	err = handler.Write(newMessage(p2p.CmdTx))
	assert.EqualError(t, err, "[HandlerEIP001] VerifyTransaction failed when AppendToTxnPool")
	response, err = handler.Read()
	assert.Equal(t, p2p.CmdReject, response.CMD())

	// message notfound
	err = handler.Write(newMessage(p2p.CmdNotFound))
	assert.NoError(t, err)

	// message mempool
	err = handler.Write(newMessage(p2p.CmdMemPool))
	assert.NoError(t, err)

	// message reject
	err = handler.Write(newMessage(p2p.CmdReject))
	assert.Error(t, err)
}
