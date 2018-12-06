package node

import (
	"crypto/rand"
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	. "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/stretchr/testify/assert"
)

func initLocalNode(t *testing.T) {
	log.Init(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	foundation, err := common.Uint168FromAddress("8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3")
	if !assert.NoError(t, err) {
		return
	}
	FoundationAddress = *foundation
	chainStore, err := NewChainStore("TestChain")
	if err != nil {
		t.Error(err.Error())
	}

	err = Init(chainStore, nil)
	if err != nil {
		t.Error(err.Error())
	}

	InitLocalNode()
}

func newTestNode(t *testing.T, port uint16) (protocol.Noder, protocol.Noder) {
	buf := make([]byte, 8)
	conn, err := NonTLSDial(fmt.Sprint("127.0.0.1:", port))
	if err != nil {
		t.FailNow()
	}
	this := NewNode(conn, false)
	rand.Read(buf)
	this.id = binary.BigEndian.Uint64(buf)

	log.Infof("Local node %s connect with %s with %s",
		conn.LocalAddr().String(), conn.RemoteAddr().String(),
		conn.RemoteAddr().Network())

	that := <-LocalNode.handshakeQueue.capChan
	LocalNode.handshakeQueue.capChan <- that
	LocalNode.RemoveFromHandshakeQueue(that)

	this.SetState(protocol.ESTABLISHED)
	that.SetState(protocol.ESTABLISHED)

	return this, that
}

func TestNode_Connect(t *testing.T) {
	initLocalNode(t)

	// handshake queue do not over flow capacity
	for i := 0; i < protocol.DefaultMaxPeers*2; i++ {
		_, err := NonTLSDial(fmt.Sprint("127.0.0.1:", config.Parameters.NodePort))
		assert.NoError(t, err)
	}
	assert.Equal(t, protocol.DefaultMaxPeers, len(LocalNode.handshakeQueue.conns))

	// handshake queue should cleaned after handshake timeout
	start := time.Now()
	for len(LocalNode.handshakeQueue.conns) > 0 {
		time.Sleep(time.Millisecond * 100)
	}
	end := time.Now()
	if end.After(start.Add(time.Second * 5)) {
		t.Errorf("Timeout duration longer than expected, actual duration %s", end.Sub(start).String())
	}
	t.Logf("Timeout duration %s", end.Sub(start).String())
}

func TestMessages(t *testing.T) {
	// Only cased message types can go through
	openMsgTypes := []string{
		p2p.CmdGetAddr,
		p2p.CmdPing,
		p2p.CmdPong,
		p2p.CmdFilterLoad,
		p2p.CmdGetBlocks,
		p2p.CmdGetData,
		p2p.CmdTx,
		p2p.CmdMemPool,
	}

	innerMsgTypes := []string{
		p2p.CmdAddr,
		p2p.CmdInv,
		p2p.CmdBlock,
		p2p.CmdMerkleBlock,
		p2p.CmdNotFound,
		p2p.CmdReject,
	}

	allMsgTypes := append(openMsgTypes, innerMsgTypes...)

	this, that := newTestNode(t, config.Parameters.NodeOpenPort)
	this.UpdateHandler(NewHandlerEIP001(this))
	that.UpdateHandler(NewHandlerEIP001(that))
	for _, cmd := range openMsgTypes {
		this.(*node).SendMessage(newMessage(cmd))
	}

	this, that = newTestNode(t, config.Parameters.NodePort)
	this.UpdateHandler(NewHandlerEIP001(this))
	that.UpdateHandler(NewHandlerEIP001(that))
	for _, cmd := range allMsgTypes {
		this.(*node).SendMessage(newMessage(cmd))
	}
}

func newMessage(cmd string) p2p.Message {
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
	case p2p.CmdInv:
		message = new(msg.Inventory)
	case p2p.CmdGetData:
		message = new(msg.GetData)
	case p2p.CmdNotFound:
		message = new(msg.NotFound)
	case p2p.CmdBlock:
		message = msg.NewBlock(new(core.Block))
	case p2p.CmdTx:
		message = msg.NewTx(NewCoinBaseTransaction(new(core.PayloadCoinBase), 0))
	case p2p.CmdPing:
		message = new(msg.Ping)
	case p2p.CmdPong:
		message = new(msg.Pong)
	case p2p.CmdMemPool:
		message = new(msg.MemPool)
	case p2p.CmdFilterLoad:
		message = new(msg.FilterLoad)
	case p2p.CmdMerkleBlock:
		message = msg.NewMerkleBlock(new(core.Header))
	case p2p.CmdReject:
		message = new(msg.Reject)
	}
	return message
}

func TestNodeDone(t *testing.T) {
	//DefaultLedger.Store.Close()
}
