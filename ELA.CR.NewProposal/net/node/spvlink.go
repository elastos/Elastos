package node

import (
	"net"
	"fmt"

	"Elastos.ELA/common/log"
	"Elastos.ELA/net/protocol"
	"Elastos.ELA/common/config"
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
