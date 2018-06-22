package node

import (
	"fmt"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg/v0"
)

type HandlerV0 struct {
	HandlerBase
	duplicateBlocks int
}

func NewHandlerV0(node protocol.Noder) *HandlerV0 {
	return &HandlerV0{HandlerBase: HandlerBase{node: node}}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerV0) OnMakeMessage(cmd string) (message p2p.Message, err error) {
	// Filter messages through open port message filter
	if err = h.FilterMessage(cmd); err != nil {
		return message, err
	}
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
	case p2p.CmdBlock:
		message = msg.NewBlock(new(core.Block))
	case p2p.CmdTx:
		message = msg.NewTx(new(core.Transaction))
	case p2p.CmdNotFound:
		message = new(v0.NotFound)
	default:
		message, err = h.HandlerBase.OnMakeMessage(cmd)
	}

	return message, err
}

func (h *HandlerV0) OnMessageDecoded(message p2p.Message) {
	if err := h.HandleMessage(message); err != nil {
		log.Error("Handle message error: " + err.Error())
	}
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *HandlerV0) HandleMessage(message p2p.Message) error {
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
	case *msg.Block:
		err = h.onBlock(message)
	case *msg.Tx:
		err = h.onTx(message)
	case *v0.NotFound:
		err = h.onNotFound(message)
	default:
		h.HandlerBase.OnMessageDecoded(message)
	}
	return err
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
	hashes, err := GetBlockHashes(*start, req.HashStop, p2p.MaxHeaderHashes)
	if err != nil {
		return err
	}

	if len(hashes) > 0 {
		go node.Send(v0.NewInv(hashes))
	}
	return nil
}

func (h *HandlerV0) onInv(inv *v0.Inv) error {
	log.Debug()
	node := h.node
	log.Debugf("[OnInv] count %d hashes: %v", len(inv.Hashes), inv.Hashes)

	if LocalNode.IsSyncHeaders() && !node.IsSyncHeaders() {
		return nil
	}

	for i, hash := range inv.Hashes {
		// Request block
		if !chain.DefaultLedger.BlockInLedger(*hash) &&
			(!chain.DefaultLedger.Blockchain.IsKnownOrphan(hash) || !LocalNode.IsRequestedBlock(*hash)) {

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

	node.Send(msg.NewBlock(block))

	return nil
}

func (h *HandlerV0) onBlock(msgBlock *msg.Block) error {
	log.Debug()
	node := h.node
	block := msgBlock.Block.(*core.Block)

	hash := block.Hash()
	if !LocalNode.IsNeighborNoder(node) {
		log.Trace("received block message from unknown peer")
		return fmt.Errorf("received block message from unknown peer")
	}

	if chain.DefaultLedger.BlockInLedger(hash) {
		h.duplicateBlocks++
		log.Trace("Receive ", h.duplicateBlocks, " duplicated block.")
		return fmt.Errorf("received duplicated block")
	}

	chain.DefaultLedger.Store.RemoveHeaderListElement(hash)
	LocalNode.DeleteRequestedBlock(hash)
	_, isOrphan, err := chain.DefaultLedger.Blockchain.AddBlock(block)
	if err != nil {
		return fmt.Errorf("Block add failed: %s ,block hash %s ", err.Error(), hash.String())
	}

	if !LocalNode.IsSyncHeaders() {
		// relay
		if !LocalNode.ExistedID(hash) {
			LocalNode.Relay(node, block)
			log.Debug("Relay block")
		}

		if isOrphan && !LocalNode.IsRequestedBlock(hash) {
			orphanRoot := chain.DefaultLedger.Blockchain.GetOrphanRoot(&hash)
			locator, _ := chain.DefaultLedger.Blockchain.LatestBlockLocator()
			SendGetBlocks(node, locator, *orphanRoot)
		}
	}

	return nil
}

func (h *HandlerV0) onTx(msgTx *msg.Tx) error {
	log.Debug()
	node := h.node
	tx := msgTx.Transaction.(*core.Transaction)

	if !LocalNode.ExistedID(tx.Hash()) && !LocalNode.IsSyncHeaders() {
		if errCode := LocalNode.AppendToTxnPool(tx); errCode != errors.Success {
			return fmt.Errorf("[HandlerBase] VerifyTransaction failed when AppendToTxnPool")
		}
		LocalNode.Relay(node, tx)
		log.Debugf("Relay Transaction hash %s type %s", tx.Hash().String(), tx.TxType.Name())
		LocalNode.IncRxTxnCnt()
	}

	return nil
}

func (h *HandlerV0) onNotFound(msg *v0.NotFound) error {
	log.Debug("Received not found message, hash: ", msg.Hash.String())
	return nil
}
