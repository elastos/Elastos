package node

import (
	"errors"
	"fmt"
	"strconv"
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
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

// NewHandlerBase create a new HandlerBase instance
func NewHandlerBase(node protocol.Noder) *HandlerBase {
	return &HandlerBase{node: node}
}

// When something wrong on read or decode message
// this method will callback the error
func (h *HandlerBase) OnError(err error) {
	switch err {
	case p2p.ErrInvalidHeader,
		p2p.ErrUnmatchedMagic,
		p2p.ErrMsgSizeExceeded:
		log.Error(err)
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
	if err := h.HandleMessage(message); err != nil {
		log.Errorf("Handle message error %s", err.Error())
	}
}

func (h *HandlerBase) HandleMessage(message p2p.Message) error {
	var err error
	switch message := message.(type) {
	case *msg.Version:
		err = h.onVersion(message)
	case *msg.VerAck:
		err = h.onVerAck(message)
	case *msg.GetAddr:
		err = h.onGetAddr(message)
	case *msg.Addr:
		err = h.onAddr(message)
	default:
		err = errors.New("unknown message type")
	}
	return err
}

func (h *HandlerBase) onVersion(version *msg.Version) error {
	node := h.node
	// Exclude the node itself
	if version.Nonce == LocalNode.ID() {
		log.Warn("The node handshake with itself")
		node.CloseConn()
		return errors.New("The node handshake with itself")
	}

	if node.State() != p2p.INIT && node.State() != p2p.HAND {
		log.Warn("unknown status to receive version")
		return errors.New("unknown status to receive version")
	}

	// Obsolete node
	n, ret := LocalNode.DelNeighborNode(version.Nonce)
	if ret == true {
		log.Info(fmt.Sprintf("Node reconnect 0x%x", version.Nonce))
		// Close the connection and release the node soure
		n.SetState(p2p.INACTIVITY)
		n.CloseConn()
	}

	node.UpdateInfo(time.Now(), version.Version, version.Services,
		version.Port, version.Nonce, version.Relay, version.Height)
	LocalNode.AddNeighborNode(node)

	// Update message handler according to the protocol version
	if version.Version < p2p.EIP001Version {
		node.UpdateMsgHelper(NewHandlerV0(node))
	} else {
		node.UpdateMsgHelper(NewHandlerEIP001(node))
	}

	// Do not add extra node address into known addresses, for this can
	// stop inner node from creating an outbound connection to extra node.
	if !node.IsFromExtraNet() {
		ip, _ := node.Addr16()
		addr := p2p.NetAddress{
			Time:     node.GetTime(),
			Services: version.Services,
			IP:       ip,
			Port:     version.Port,
			ID:       version.Nonce,
		}
		LocalNode.AddKnownAddress(addr)
	}

	var message p2p.Message
	if node.State() == p2p.INIT {
		node.SetState(p2p.HANDSHAKE)
		version := NewVersion(LocalNode)
		if node.IsFromExtraNet() {
			version.Port = config.Parameters.NodeOpenPort
		} else {
			version.Port = config.Parameters.NodePort
		}
		message = version
	} else if node.State() == p2p.HAND {
		node.SetState(p2p.HANDSHAKED)
		message = new(msg.VerAck)
	}
	node.Send(message)

	return nil
}

func (h *HandlerBase) onVerAck(verAck *msg.VerAck) error {
	node := h.node
	if node.State() != p2p.HANDSHAKE && node.State() != p2p.HANDSHAKED {
		log.Warn("unknown status to received verack")
		return errors.New("unknown status to received verack")
	}

	if node.State() == p2p.HANDSHAKE {
		node.Send(verAck)
	}

	node.SetState(p2p.ESTABLISH)

	if LocalNode.NeedMoreAddresses() {
		node.RequireNeighbourList()
	}
	addr := node.Addr()
	port := node.Port()
	nodeAddr := addr + ":" + strconv.Itoa(int(port))
	LocalNode.RemoveFromHandshakeQueue(node)
	LocalNode.RemoveFromConnectingList(nodeAddr)
	return nil
}

func (h *HandlerBase) onGetAddr(getAddr *msg.GetAddr) error {
	var addrs []p2p.NetAddress
	// Only send addresses that enabled SPV service
	if h.node.IsFromExtraNet() {
		for _, addr := range LocalNode.RandSelectAddresses() {
			if addr.Services&protocol.OpenService == protocol.OpenService {
				addr.Port = config.Parameters.NodeOpenPort
				addrs = append(addrs, addr)
			}
		}
	} else {
		addrs = LocalNode.RandSelectAddresses()
	}

	h.node.Send(msg.NewAddr(addrs))
	return nil
}

func (h *HandlerBase) onAddr(msgAddr *msg.Addr) error {
	for _, addr := range msgAddr.AddrList {
		log.Debugf("The ip address is %s id is 0x%x", addr.String(), addr.ID)

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
		LocalNode.AddKnownAddress(addr)
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
