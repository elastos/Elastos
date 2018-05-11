package node

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/protocol"
)

// SPV client will connect node through the specific spv port
// only SPV messages can go through, such as filterload/getblocks/getdata etc.
// Otherwise error will be returned
func FilterMessage(node protocol.Noder, msgType string) error {
	if node.LocalPort() == protocol.SPVPort {
		// Only cased message types can go through
		switch msgType {
		case "version":
		case "verack":
		case "getaddr":
		case "addr":
			return fmt.Errorf("message addr can not go through SPV port")
		case "filterload":
		case "getblocks":
		case "getdata":
		case "ping":
		case "pong":
		case "tx":
		default:
			return fmt.Errorf("unsupport messsage from SPV port %d, type: %s", node.Port(), msgType)
		}
	} else {
		// Node not using SPV port can not send filterload message
		switch msgType {
		case "filterload":
			return fmt.Errorf("SPV messsage from non SPV port %d, type: %s", node.Port(), msgType)
		}
	}
	return nil
}
