package node

import (
	"errors"
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

var _ protocol.Handler = (*HandlerBase)(nil)

type HandlerBase struct {
	node protocol.Noder
}

// NewHandlerBase create a new HandlerBase instance
func NewHandlerBase(node protocol.Noder) *HandlerBase {
	return &HandlerBase{node: node}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerBase) MakeEmptyMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case p2p.CmdVersion:
		message = &msg.Version{}

	case p2p.CmdVerAck:
		message = &msg.VerAck{}

	case p2p.CmdGetAddr:
		message = &msg.GetAddr{}

	case p2p.CmdAddr:
		message = &msg.Addr{}

	case p2p.CmdPing:
		message = &msg.Ping{}

	case p2p.CmdPong:
		message = &msg.Pong{}

	default:
		err = errors.New("unknown message type")
	}

	return message, err
}

func (h *HandlerBase) HandleMessage(message p2p.Message) {
	switch m := message.(type) {
	case *msg.Version:
		h.onVersion(m)

	case *msg.VerAck:
		h.onVerAck(m)

	case *msg.GetAddr:
		h.onGetAddr(m)

	case *msg.Addr:
		h.onAddr(m)

	case *msg.Ping:
		h.onPing(m)

	case *msg.Pong:
		h.onPong(m)

	default:
		log.Warnf("unknown handled message %s", m.CMD())
	}
}

func (h *HandlerBase) onVersion(version *msg.Version) {
	node := h.node
	// Exclude the node itself
	if version.Nonce == LocalNode.ID() {
		log.Warn("The node handshake with itself")
		node.Disconnect()
		return
	}

	if node.State() != protocol.INIT && node.State() != protocol.HAND {
		log.Warn("unknown status to receive version")
		node.Disconnect()
		return
	}

	//// Obsolete node
	//n, ret := LocalNode.DelNeighborNode(version.Nonce)
	//if ret == true {
	//	log.Info(fmt.Sprintf("Node %s reconnect", n))
	//	// Close the connection and release the node soure
	//	n.Disconnect()
	//}

	node.UpdateInfo(time.Unix(int64(version.TimeStamp), 0), version.Version,
		version.Services, version.Port, version.Nonce, version.Relay, version.Height)

	// Update message handler according to the protocol version
	if version.Version < p2p.EIP001Version {
		node.UpdateHandler(NewHandlerV0(node))
	} else {
		node.UpdateHandler(NewHandlerEIP001(node))
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
		message = &msg.VerAck{}
	}
	node.SendMessage(message)
}

func (h *HandlerBase) onVerAck(verAck *msg.VerAck) {
	node := h.node
	if node.State() != protocol.HANDSHAKE && node.State() != protocol.HANDSHAKED {
		log.Warn("unknown status to received verack")
		node.Disconnect()
		return
	}

	if node.State() == protocol.HANDSHAKE {
		node.SendMessage(verAck)
	}

	node.SetState(protocol.ESTABLISHED)

	// Finish handshake
	LocalNode.RemoveFromHandshakeQueue(node)
	LocalNode.RemoveFromConnectingList(node.Addr())

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
}

func (h *HandlerBase) onPing(ping *msg.Ping) {
	log.Debug("onPing")
	h.node.SetHeight(ping.Nonce)
	h.node.SendMessage(msg.NewPong(uint64(chain.DefaultLedger.Store.GetHeight())))
}

func (h *HandlerBase) onPong(pong *msg.Pong) {
	log.Debug("onPong")
	h.node.SetHeight(pong.Nonce)
}

func (h *HandlerBase) onGetAddr(getAddr *msg.GetAddr) {
	var addrs []*p2p.NetAddress
	// Only send addresses that enabled SPV service
	if h.node.IsExternal() {
		for _, addr := range LocalNode.RandSelectAddresses() {
			if addr.Services&protocol.OpenService == protocol.OpenService {
				addrs = append(addrs,
					&p2p.NetAddress{
						addr.Timestamp,
						addr.Services,
						addr.IP,
						config.Parameters.NodeOpenPort,
				})
			}
		}
	} else {
		addrs = LocalNode.RandSelectAddresses()
	}

	repeatNum := 0
	var uniqueAddrs []*p2p.NetAddress
	for _, addr := range addrs {
		// do not send client's address to the client itself
		if h.node.NetAddress().String() != addr.String() {
			uniqueAddrs = append(uniqueAddrs, addr)
		} else {
			repeatNum ++
			if repeatNum > 1 {
				log.Warn("more than one repeat:", repeatNum," ", repeatNum, " ", addr.String())
			}

		}
	}

	if len(uniqueAddrs) > 0 {
		h.node.SendMessage(msg.NewAddr(addrs))
	}
}

func (h *HandlerBase) onAddr(msgAddr *msg.Addr) {
	if h.node.IsExternal() {
		// we don't accept address list from a external node/spv...etc.
		return
	}
	for _, addr := range msgAddr.AddrList {
		if addr.Port == 0 {
			continue
		}
		//save the node address in address list
		LocalNode.AddKnownAddress(addr)
	}
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
	node.SendMessage(msg.NewGetBlocks(locator, hashStop))
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
