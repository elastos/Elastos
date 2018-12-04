package node

import (
	"testing"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg/v0"
	"github.com/stretchr/testify/assert"
)

/*
This test is for the p2p connections come from external net.
There are several rules in open connections:
1. will be marked as from external net
2. can not send addr message
3. can not send inventory type block message
4. can not send block message
5. can not send notfound message
6. can not send reject message
*/

func TestOpenConnectionInit(t *testing.T) {
	config.Parameters.OpenService = true
	config.Parameters.NodeOpenPort = 20866
	initLocalNode(t)
}

func TestV0Messages(t *testing.T) {
	handler := newTestHandlerV0(t, config.Parameters.NodeOpenPort)

	// check if the node is marked as external node
	assert.Equal(t, true, handler.that.IsExternal())

	// addr message should be rejected
	err := handler.Write(newMessage(p2p.CmdAddr))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [addr] from external node")

	// inventory message type block should be rejected
	err = handler.Write(v0.NewInv([]*common.Uint256{&common.EmptyHash}))
	assert.EqualError(t, err, "receive inv message from external node")

	// block message should be rejected
	err = handler.Write(newMessage(p2p.CmdBlock))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [block] from external node")

	// notfound message should be rejected
	err = handler.Write(newMessage(p2p.CmdNotFound))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [notfound] from external node")
}

func TestEIP001Messages(t *testing.T) {
	handler := newTestHandlerEIP001(t, config.Parameters.NodeOpenPort)

	// check if the node is marked as external node
	assert.Equal(t, true, handler.that.IsExternal())

	// addr message should be rejected
	err := handler.Write(newMessage(p2p.CmdAddr))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [addr] from external node")

	// inventory message type block should be rejected
	inv := msg.NewInventory()
	inv.AddInvVect(msg.NewInvVect(msg.InvTypeBlock, &common.EmptyHash))
	err = handler.Write(inv)
	assert.EqualError(t, err, "receive InvTypeBlock from external node")

	// block message should be rejected
	err = handler.Write(newMessage(p2p.CmdBlock))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [block] from external node")

	// notfound message should be rejected
	err = handler.Write(newMessage(p2p.CmdNotFound))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [notfound] from external node")

	// reject message should be rejected
	err = handler.Write(newMessage(p2p.CmdReject))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [reject] from external node")
}
