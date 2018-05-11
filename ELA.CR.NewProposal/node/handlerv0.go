package node

import (
	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg/v0"
)

type HandlerV0 struct {
	HandlerBase
}

func NewHandlerV0(node protocol.Noder) *HandlerV0 {
	return &HandlerV0{HandlerBase{node}}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerV0) OnMakeMessage(cmd string) (message p2p.Message, err error) {

	// Update node last active time
	h.node.UpdateLastActive()

	switch cmd {
	case p2p.CmdPing:
		message = new(msg.Ping)
	case p2p.CmdPong:
		message = new(msg.Pong)
	case p2p.CmdGetBlocks:
		message = new(msg.GetBlocks)
	case p2p.CmdInv:
		message = new(v0.Inv)
	case p2p.CmdGetData:
		message = new(v0.GetData)
	case p2p.CmdNotFound:
		message = new(v0.NotFound)
	default:
		message, err = h.HandlerBase.OnMakeMessage(cmd)
	}

	return message, err
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *HandlerV0) OnMessageDecoded(message p2p.Message) {
	var err error
	switch message := message.(type) {
	case *msg.Ping:
		err = h.onPing(message)
	case *msg.Pong:
		err = h.onPong(message)
	case *msg.GetBlocks:
		err = h.onGetBlocks(message)
	case *v0.Inv:
		err = h.onInv(message)
	case *v0.GetData:
		err = h.onGetData(message)
	case *v0.NotFound:
		err = h.onNotFound(message)
	default:
		h.HandlerBase.OnMessageDecoded(message)
	}
	if err != nil {
		log.Error("Handler message error: " + err.Error())
	}
}

func (h *HandlerV0) onPing(ping *msg.Ping) error {
	log.Debug()
	h.node.SetHeight(ping.Nonce)
	h.node.Send(msg.NewPong(chain.DefaultLedger.Store.GetHeight()))
	return nil
}

func (h *HandlerV0) onPong(pong *msg.Pong) error {
	log.Debug()
	h.node.SetHeight(pong.Nonce)
	return nil
}

func (h *HandlerV0) onGetBlocks(req *msg.GetBlocks) error {
	log.Debug()
	node := h.node
	LocalNode.AcqSyncHdrReqSem()
	defer LocalNode.RelSyncHdrReqSem()

	start := chain.DefaultLedger.Blockchain.LatestLocatorHash(req.Locator)
	hashes, err := GetBlockHashes(*start, req.HashStop)
	if err != nil {
		return err
	}
	node.Send(v0.NewInv(hashes))
	return nil
}

func (h *HandlerV0) onInv(inv *v0.Inv) error {
	log.Debug()
	node := h.node
	log.Debugf("[OnInv] count %d hashes: %v\n", len(inv.Hashes), inv.Hashes)

	if LocalNode.IsSyncHeaders() && !node.IsSyncHeaders() {
		return nil
	}

	for i, hash := range inv.Hashes {
		// Request block
		if !chain.DefaultLedger.BlockInLedger(*hash) &&
			(!chain.DefaultLedger.Blockchain.IsKnownOrphan(hash) || !LocalNode.RequestedBlockExisted(*hash)) {

			LocalNode.AddRequestedBlock(*hash)
			node.Send(v0.NewGetData(*hash))
		}

		// Request fork chain
		if chain.DefaultLedger.Blockchain.IsKnownOrphan(hash) {
			orphanRoot := chain.DefaultLedger.Blockchain.GetOrphanRoot(hash)
			locator, err := chain.DefaultLedger.Blockchain.LatestBlockLocator()
			if err != nil {
				log.Errorf("Failed to get block locator for the latest block: %v", err)
				continue
			}
			SendGetBlocks(node, locator, *orphanRoot)
			continue
		}

		// Request next hashes
		if i == len(inv.Hashes)-1 {
			locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(hash)
			SendGetBlocks(node, locator, common.EmptyHash)
		}
	}
	return nil
}

func (h *HandlerV0) onGetData(req *v0.GetData) error {
	log.Debug()
	node := h.node
	hash := req.Hash

	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		log.Debug("Can't get block from hash: ", hash, " ,send not found message")
		node.Send(v0.NewNotFound(hash))
		return err
	}
	log.Debug("block height is ", block.Header.Height, " ,hash is ", hash)

	node.Send(msg.NewBlock(block))

	return nil
}

func (h *HandlerV0) onNotFound(msg *v0.NotFound) error {
	log.Debug("Received not found message, hash: ", msg.Hash.String())
	return nil
}
