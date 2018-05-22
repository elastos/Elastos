package node

import (
	"errors"
	"fmt"
	"strconv"
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type HandlerBase struct {
	node protocol.Noder
}

// When something wrong on read or decode message
// this method will callback the error
func (h *HandlerBase) OnError(err error) {
	switch err {
	case p2p.ErrUnmatchedMagic:
		log.Error("[MsgHandler] unmatched magic")
		h.node.CloseConn()
	case p2p.ErrDisconnected:
		log.Error("[MsgHandler] connection disconnected")
		LocalNode.GetEvent("disconnect").Notify(events.EventNodeDisconnect, h.node)
	default:
		log.Error(err)
	}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerBase) OnMakeMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case p2p.CmdVersion:
		message = new(msg.Version)
	case p2p.CmdVerAck:
		message = new(msg.VerAck)
	case p2p.CmdGetAddr:
		message = new(msg.GetAddr)
	case p2p.CmdAddr:
		message = new(msg.Addr)
	default:
		err = errors.New("unknown message type")
	}

	return message, err
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *HandlerBase) OnMessageDecoded(message p2p.Message) {
	var err error
	switch message := message.(type) {
	case *msg.Version:
		err = h.onVersion(message)
	case *msg.VerAck:
		err = h.onVerAck(message)
	case *msg.GetAddr:
		err = h.onAddrsReq(message)
	case *msg.Addr:
		err = h.onAddrs(message)
	default:
		err = errors.New("unknown message type")
	}
	if err != nil {
		log.Error("Handler message error: " + err.Error())
	}
}

func (h *HandlerBase) onVersion(version *msg.Version) error {
	node := h.node

	// Exclude the node itself
	if version.Nonce == LocalNode.ID() {
		log.Warn("The node handshake with itself")
		node.CloseConn()
		return errors.New("The node handshake with itself")
	}

	s := node.State()
	if s != p2p.INIT && s != p2p.HAND {
		log.Warn("Unknown status to receive version")
		return errors.New("Unknown status to receive version")
	}

	// Obsolete node
	n, ret := LocalNode.DelNbrNode(version.Nonce)
	if ret == true {
		log.Info(fmt.Sprintf("Node reconnect 0x%x", version.Nonce))
		// Close the connection and release the node soure
		n.SetState(p2p.INACTIVITY)
		n.CloseConn()
	}

	node.UpdateInfo(time.Now(), version.Version, version.Services,
		version.Port, version.Nonce, version.Relay, version.Height)
	LocalNode.AddNbrNode(node)

	// Update message handler according to the protocol version
	if version.Version < p2p.EIP001Version {
		node.UpdateMsgHelper(NewHandlerV0(LocalNode))
	} else {
		node.UpdateMsgHelper(NewHandlerEIP001(node))
	}

	// Do not add SPV client as a known address,
	// so node will not start a connection to SPV client
	if node.LocalPort() != protocol.SPVPort {
		ip, _ := node.Addr16()
		addr := p2p.NetAddress{
			Time:     node.GetTime(),
			Services: version.Services,
			IP:       ip,
			Port:     version.Port,
			ID:       version.Nonce,
		}
		LocalNode.AddAddressToKnownAddress(addr)
	}

	var message p2p.Message
	if s == p2p.INIT {
		node.SetState(p2p.HANDSHAKE)
		message = NewVersion(LocalNode)
	} else if s == p2p.HAND {
		node.SetState(p2p.HANDSHAKED)
		message = new(msg.VerAck)
	}
	node.Send(message)

	return nil
}

func (h *HandlerBase) onVerAck(verAck *msg.VerAck) error {
	node := h.node
	s := node.State()
	if s != p2p.HANDSHAKE && s != p2p.HANDSHAKED {
		log.Warn("unknown status to received verack")
		return errors.New("unknown status to received verack")
	}

	node.SetState(p2p.ESTABLISH)

	if s == p2p.HANDSHAKE {
		node.Send(verAck)
	}

	if LocalNode.NeedMoreAddresses() {
		node.ReqNeighborList()
	}
	addr := node.Addr()
	port := node.Port()
	nodeAddr := addr + ":" + strconv.Itoa(int(port))
	LocalNode.RemoveAddrInConnectingList(nodeAddr)
	return nil
}

func (h *HandlerBase) onAddrsReq(req *msg.GetAddr) error {
	addrs := LocalNode.RandSelectAddresses()
	h.node.Send(msg.NewAddr(addrs))
	return nil
}

func (h *HandlerBase) onAddrs(addrs *msg.Addr) error {
	for _, addr := range addrs.AddrList {
		log.Info(fmt.Sprintf("The ip address is %s id is 0x%x", addr.String(), addr.ID))

		if addr.ID == LocalNode.ID() {
			continue
		}
		if LocalNode.NodeEstablished(addr.ID) {
			continue
		}

		if addr.Port == 0 {
			continue
		}

		//save the node address in address list
		LocalNode.AddAddressToKnownAddress(addr)
	}
	return nil
}

func NewVersion(node protocol.Noder) *msg.Version {
	msg := new(msg.Version)
	msg.Version = node.Version()
	msg.Services = node.Services()

	msg.TimeStamp = uint32(time.Now().UTC().UnixNano())
	msg.Port = node.Port()
	msg.Nonce = node.ID()
	msg.Height = uint64(chain.DefaultLedger.GetLocalBlockChainHeight())
	if node.IsRelay() {
		msg.Relay = 1
	} else {
		msg.Relay = 0
	}

	return msg
}

func SendGetBlocks(node protocol.Noder, locator []*common.Uint256, hashStop common.Uint256) {
	if LocalNode.GetStartHash() == *locator[0] && LocalNode.GetStopHash() == hashStop {
		return
	}

	LocalNode.SetSyncHeaders(true)
	node.SetSyncHeaders(true)
	LocalNode.SetStartHash(*locator[0])
	LocalNode.SetStopHash(hashStop)
	node.Send(msg.NewGetBlocks(locator, hashStop))
}

func GetBlockHashes(startHash common.Uint256, stopHash common.Uint256, maxBlockHashes uint32) ([]*common.Uint256, error) {
	var count = uint32(0)
	var startHeight uint32
	var stopHeight uint32
	curHeight := chain.DefaultLedger.Store.GetHeight()
	if stopHash == common.EmptyHash {
		if startHash == common.EmptyHash {
			if curHeight > maxBlockHashes {
				count = maxBlockHashes
			} else {
				count = curHeight
			}
		} else {
			startHeader, err := chain.DefaultLedger.Store.GetHeader(startHash)
			if err != nil {
				return nil, err
			}
			startHeight = startHeader.Height
			count = curHeight - startHeight
			if count > maxBlockHashes {
				count = maxBlockHashes
			}
		}
	} else {
		stopHeader, err := chain.DefaultLedger.Store.GetHeader(stopHash)
		if err != nil {
			return nil, err
		}
		stopHeight = stopHeader.Height
		if startHash != common.EmptyHash {
			startHeader, err := chain.DefaultLedger.Store.GetHeader(startHash)
			if err != nil {
				return nil, err
			}
			startHeight = startHeader.Height

			// avoid unsigned integer underflow
			if stopHeight < startHeight {
				return nil, errors.New("do not have header to send")
			}
			count = stopHeight - startHeight

			if count >= maxBlockHashes {
				count = maxBlockHashes
			}
		} else {
			if stopHeight > maxBlockHashes {
				count = maxBlockHashes
			} else {
				count = stopHeight
			}
		}
	}

	hashes := make([]*common.Uint256, 0)
	for i := uint32(1); i <= count; i++ {
		hash, err := chain.DefaultLedger.Store.GetBlockHash(startHeight + i)
		if err != nil {
			return nil, err
		}
		hashes = append(hashes, &hash)
	}

	return hashes, nil
}
