package node

import (
	"errors"
	"fmt"
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
		LocalNode.Events().Notify(events.EventNodeDisconnect, h.node.ID())
	default:
		log.Error(err)
	}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerBase) OnMakeMessage(cmd string) (message p2p.Message, err error) {
	// Nothing to do if node already disconnected
	if h.node.State() == protocol.INACTIVITY {
		return message, fmt.Errorf("revice message from INACTIVE node [0x%x]", h.node.ID())
	}

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
	log.Debugf("-----> [%s] from peer [0x%x] STARTED", message.CMD(), h.node.ID())
	if err := h.HandleMessage(message); err != nil {
		log.Error("Handle message error: " + err.Error())
	}
	log.Debugf("-----> [%s] from peer [0x%x] FINISHED", message.CMD(), h.node.ID())
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

	if node.State() != protocol.INIT && node.State() != protocol.HAND {
		log.Warn("unknown status to receive version")
		return errors.New("unknown status to receive version")
	}

	// Obsolete node
	n, ret := LocalNode.DelNeighborNode(version.Nonce)
	if ret == true {
		log.Info(fmt.Sprintf("Node reconnect 0x%x", version.Nonce))
		// Close the connection and release the node soure
		n.SetState(protocol.INACTIVITY)
		n.CloseConn()
	}

	node.UpdateInfo(time.Now(), version.Version, version.Services,
		version.Port, version.Nonce, version.Relay, version.Height)

	// Update message handler according to the protocol version
	if version.Version < p2p.EIP001Version {
		node.UpdateMsgHelper(NewHandlerV0(node))
	} else {
		node.UpdateMsgHelper(NewHandlerEIP001(node))
	}

	var message p2p.Message
	if node.State() == protocol.INIT {
		node.SetState(protocol.HANDSHAKE)
		version := NewVersion(LocalNode)
		// External node connect with open port
		if node.IsExternal() {
			version.Port = config.Parameters.NodeOpenPort
		} else {
			version.Port = config.Parameters.NodePort
		}
		message = version
	} else if node.State() == protocol.HAND {
		node.SetState(protocol.HANDSHAKED)
		message = new(msg.VerAck)
	}
	node.Send(message)

	return nil
}

func (h *HandlerBase) onVerAck(verAck *msg.VerAck) error {
	node := h.node
	if node.State() != protocol.HANDSHAKE && node.State() != protocol.HANDSHAKED {
		log.Warn("unknown status to received verack")
		return errors.New("unknown status to received verack")
	}

	if node.State() == protocol.HANDSHAKE {
		node.Send(verAck)
	}

	node.SetState(protocol.ESTABLISHED)

	// Finish handshake
	LocalNode.RemoveFromHandshakeQueue(node)
	LocalNode.RemoveFromConnectingList(node.NetAddress().String())

	// Add node to neighbor list
	LocalNode.AddNeighborNode(node)

	// Do not add external node address into known addresses, for this can
	// stop internal node from creating an outbound connection to it.
	if !node.IsExternal() {
		LocalNode.AddKnownAddress(node.NetAddress())
	}

	// Request more neighbor addresses
	if LocalNode.NeedMoreAddresses() {
		node.RequireNeighbourList()
	}

	// Start heartbeat
	go node.Heartbeat()

	return nil
}

func (h *HandlerBase) onGetAddr(getAddr *msg.GetAddr) error {
	var addrs []p2p.NetAddress
	// Only send addresses that enabled SPV service
	if h.node.IsExternal() {
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
	return &msg.Version{
		Version:   node.Version(),
		Services:  node.Services(),
		TimeStamp: uint32(time.Now().Unix()),
		Port:      node.Port(),
		Nonce:     node.ID(),
		Height:    uint64(chain.DefaultLedger.GetLocalBlockChainHeight()),
		Relay:     node.IsRelay(),
	}
}

func SendGetBlocks(node protocol.Noder, locator []*common.Uint256, hashStop common.Uint256) {
	if LocalNode.GetStartHash() == *locator[0] && LocalNode.GetStopHash() == hashStop {
		return
	}

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
