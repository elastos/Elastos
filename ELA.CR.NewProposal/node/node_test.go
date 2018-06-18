package node

import (
	"fmt"
	"testing"
	"time"

	. "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/stretchr/testify/assert"
)

func TestInitLocalNode(t *testing.T) {
	log.Init(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	foundation, err := common.Uint168FromAddress("8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta")
	if !assert.NoError(t, err) {
		return
	}
	FoundationAddress = *foundation
	chainStore, err := NewChainStore()
	if err != nil {
		t.Error(err.Error())
	}
	defer chainStore.Close()

	err = Init(chainStore)
	if err != nil {
		t.Error(err.Error())
	}

	InitLocalNode()
}

func TestNode_Connect(t *testing.T) {
	for i := 0; i < protocol.DefaultMaxPeers+10; i++ {
		_, err := NonTLSDial(fmt.Sprint("127.0.0.1:", config.Parameters.NodePort))
		assert.NoError(t, err)
	}
	if len(LocalNode.handshakeQueue.conns) > protocol.DefaultMaxPeers {
		t.Errorf("InboundQueue outof capacity %d", len(LocalNode.handshakeQueue.conns))
	}
	time.Sleep(time.Second * protocol.HandshakeTimeout)
}

func TestMessages(t *testing.T) {
	// Only cased message types can go through
	openMsgTypes := []string{
		p2p.CmdVersion,
		p2p.CmdVerAck,
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

	conn, err := NonTLSDial(fmt.Sprint("127.0.0.1:", config.Parameters.NodePort))
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	node := NewNode(config.Parameters.Magic, conn)
	node.UpdateMsgHelper(NewHandlerEIP001(node))
	for _, cmd := range allMsgTypes {
		node.Write(newMessage(cmd))
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
		message = msg.NewTx(new(core.Transaction))
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
