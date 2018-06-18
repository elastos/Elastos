package node

import (
	"fmt"
	"net"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
)

func (node *node) listenNodeOpenPort() {
	listener, err := net.Listen("tcp", fmt.Sprint(":", config.Parameters.NodeOpenPort))
	if err != nil {
		log.Error("Error listening [NodeOpenPort] error:%s\n", err.Error())
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
