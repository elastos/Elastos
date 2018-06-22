package node

import (
	"testing"

	"github.com/elastos/Elastos.ELA/config"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/stretchr/testify/assert"
)

/*
This test is for the p2p connections come from the extra net.
There are several rules in open connections:
1. will be marked as from extra net
2. can not send addr message
3. can not send inventory message
4. can not send block message
5. can not send notfound message
6. can not send reject message
*/

func TestOpenConnectionInit(t *testing.T) {
	config.Parameters.OpenService = true
	config.Parameters.NodeOpenPort = 20866
	initLocalNode(t)
}

func TestInvalidMessages(t *testing.T) {
	handler := newTestHandlerEIP001(t, config.Parameters.NodeOpenPort)

	// check if the node is marked as extra node
	assert.Equal(t, true, handler.that.IsFromExtraNet())

	// addr message should be rejected
	err := handler.Write(newMessage(p2p.CmdAddr))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [addr] from extra node")

	// inventory message should be rejected
	err = handler.Write(newMessage(p2p.CmdInv))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [inv] from extra node")

	// block message should be rejected
	err = handler.Write(newMessage(p2p.CmdBlock))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [block] from extra node")

	// notfound message should be rejected
	err = handler.Write(newMessage(p2p.CmdNotFound))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [notfound] from extra node")

	// reject message should be rejected
	err = handler.Write(newMessage(p2p.CmdReject))
	assert.EqualError(t, err, "[MsgHelper] make message failed unsupported messsage type [reject] from extra node")
}
