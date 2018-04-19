package node

import (
	"net"
	"fmt"

	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/net/protocol"
	"github.com/elastos/Elastos.ELA/config"
)

func (node *node) listenSPVPort() {
	if config.Parameters.SPVService {
		listener, err := net.Listen("tcp", fmt.Sprint(":", protocol.SPVPort))
		if err != nil {
			log.Error("Error listening SPV port error:%s\n", err.Error())
			return
		}

		node.listenConnections(listener)
	}
}
