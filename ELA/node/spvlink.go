package node

import (
	"fmt"
	"net"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
)

func (node *node) listenSPVPort() {
	if config.Parameters.SPVService {
		listener, err := net.Listen("tcp", fmt.Sprint(":", config.Parameters.SPVNodePort))
		if err != nil {
			log.Error("Error listening SPV port error:%s\n", err.Error())
			return
		}

		node.listenConnections(listener)
	}
}
