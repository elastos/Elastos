package node

import (
	"github.com/elastos/Elastos.ELA/net/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/bloom"
	"errors"
	"github.com/elastos/Elastos.ELA/log"
)

type MsgHandler struct {
	node protocol.Noder
}

// When something wrong on read or decode message
// this method will callback the error
func (h *MsgHandler) OnDecodeError(err error) {

}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *MsgHandler) OnMakeMessage(cmd string) (p2p.Message, error) {
	err := FilterMessage(h.node, cmd)
	if err != nil {
		return nil, err
	}

	var message p2p.Message
	switch cmd {
	case "version":
		message = new(msg.Version)
	case "verack":
		message = new(msg.VerAck)
	case "getaddr":
		message = new(msg.AddrsReq)
	case "addr":
		message = new(msg.Addrs)
	case "filterload":
		message = new(bloom.FilterLoad)
	case "inv":
		message = new(msg.Inventory)
	case "getdata":
		message = new(msg.DataReq)
	case "block":
		message = new(msg.Block)
	case "tx":
		message = new(msg.Tx)
	case "getblocks":
		message = new(msg.BlocksReq)
	case "notfound":
		message = new(msg.NotFound)
	case "ping":
		message = new(msg.Ping)
	case "pong":
		message = new(msg.Pong)
	default:
		return nil, errors.New("unknown message type")
	}

	return message, nil
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *MsgHandler) OnMessageDecoded(msg p2p.Message) {
	var err error
	switch message := msg.(type) {
	case *msg.Version:
		err = h.onVersion(message)
	case *msg.VerAck:
		err = h.onVerAck(message)
	case *msg.AddrsReq:
		err = h.onAddrsReq(message)
	case *msg.Addrs:
		err = h.onAddrs(message)
	case *bloom.FilterLoad:
		err = h.onFilterLoad(message)
	case *msg.Inventory:
		err = h.onInventory(message)
	case *msg.DataReq:
		err = h.onDataReq(message)
	case *msg.Block:
		err = h.onBlock(message)
	case *msg.Tx:
		err = h.onTx(message)
	case *msg.BlocksReq:
		err = h.onBlocksReq(message)
	case *msg.NotFound:
		err = h.onNotFound(message)
	case *msg.Ping:
		err = h.onPing(message)
	case *msg.Pong:
		err = h.onPong(message)
	default:
		err = errors.New("unknown message type")
	}
	if err != nil {
		log.Error("Decode message error: " + err.Error())
	}
}

func (h *MsgHandler) onVersion(msg *msg.Version) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onVerAck(msg *msg.VerAck) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onAddrsReq(msg *msg.AddrsReq) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onAddrs(msg *msg.Addrs) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onFilterLoad(msg *bloom.FilterLoad) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onInventory(msg *msg.Inventory) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onDataReq(msg *msg.DataReq) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onBlock(msg *msg.Block) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onTx(msg *msg.Tx) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onBlocksReq(msg *msg.BlocksReq) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onNotFound(msg *msg.NotFound) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onPing(msg *msg.Ping) error {
	log.Debug()
	node := h.node
}

func (h *MsgHandler) onPong(msg *msg.Pong) error {
	log.Debug()
	node := h.node
}
