package node

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// SPV client will connect node through the specific spv port
// only SPV messages can go through, such as filterload/getblocks/getdata etc.
// Otherwise error will be returned
func FilterMessage(node protocol.Noder, msgType string) error {
	if node.LocalPort() == config.Parameters.SPVNodePort {
		// Only cased message types can go through
		switch msgType {
		case p2p.CmdVersion:
		case p2p.CmdVerAck:
		case p2p.CmdGetAddr:
		case p2p.CmdPing:
		case p2p.CmdPong:
		case p2p.CmdFilterLoad:
		case p2p.CmdGetBlocks:
		case p2p.CmdGetData:
		case p2p.CmdTx:
		case p2p.CmdMemPool:
		default:
			return fmt.Errorf("unsupport messsage from SPV port %d, type: %s", node.Port(), msgType)
		}
	} else {
		// Node not using SPV port can not send filterload message
		switch msgType {
		case p2p.CmdFilterLoad:
			return fmt.Errorf("SPV messsage from non SPV port %d, type: %s", node.Port(), msgType)
		}
	}
	return nil
}
