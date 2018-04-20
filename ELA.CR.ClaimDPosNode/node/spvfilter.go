package node

import (
	"errors"
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
		case "filterload":
		case "getblocks":
		case "getdata":
		case "getaddr":
		case "addr":
		case "ping":
		case "pong":
		case "tx":
		default:
			return errors.New("Unsupport messsage from SPV port, type:" + msgType)
		}
	} else {
		// Node not using SPV port can not send filterload message
		switch msgType {
		case "filterload":
			return errors.New("SPV messsage from non SPV port, type:" + msgType)
		}
	}
	return nil
}
