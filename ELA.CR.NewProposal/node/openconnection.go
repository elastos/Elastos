/*
Open connections are the connections came from the open service port configured
by NodeOpenPort parameter. This port is used for nodes from the extra net.
The different between an inner node and an extra node is that the messages
from an extra node are filtered. That makes an extra node can sync blocks and
transaction from an inner node, but an inner node will not sync data from an
extra node. And also, an extra node can send an transaction to inner node,
but can not send a block to inner node.

There are several rules between inner node and extra node.
1. Inner node do not start an outbound connection to an extra node.
2. Inner node do not receive Addrs message from an extra node.
3. Inner node do not receive Inv type block message from an extra node.
4. Inner node do not receive Block message from an extra node.
5. Inner node do not send non-OpenServers inner nodes to an extra node in Addrs message.
*/

package node

import (
	"fmt"
	"net"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// listen the NodeOpenPort to accept connections from extra net.
func listenNodeOpenPort() {
	listener, err := net.Listen("tcp", fmt.Sprint(":", config.Parameters.NodeOpenPort))
	if err != nil {
		log.Errorf("Error listening [NodeOpenPort] error:%s", err.Error())
		return
	}

	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Error("Error accepting ", err.Error())
			return
		}
		log.Infof("Remote node %v connect with %v", conn.RemoteAddr(), conn.LocalAddr())

		node := NewNode(config.Parameters.Magic, conn)
		node.addr, err = parseIPaddr(conn.RemoteAddr().String())
		node.fromExtraNet = true
		node.Read()
		LocalNode.AddToHandshakeQueue(node)
	}
}

// Extra net nodes will connect through the specific NodeOpenPort
// only limited messages can go through, such as filterload/getblocks/getdata etc.
// Otherwise error will be returned
func (h *HandlerBase) FilterMessage(msgType string) error {
	if h.node.IsFromExtraNet() {
		// Only cased message types can go through
		switch msgType {
		case p2p.CmdVersion:
		case p2p.CmdVerAck:
		case p2p.CmdGetAddr:
		case p2p.CmdPing:
		case p2p.CmdPong:
		case p2p.CmdFilterLoad:
		case p2p.CmdGetBlocks:
		case p2p.CmdInv:
		case p2p.CmdGetData:
		case p2p.CmdTx:
		case p2p.CmdMemPool:
		default:
			return fmt.Errorf("unsupported messsage type [%s] from extra node", msgType)
		}
	}
	return nil
}
