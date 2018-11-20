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

var _ protocol.Handler = (*HandlerV0)(nil)

type HandlerV0 struct {
	base            HandlerBase
	duplicateBlocks int
}

func NewHandlerV0(node protocol.Noder) *HandlerV0 {
	return &HandlerV0{base: HandlerBase{node: node}}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerV0) MakeEmptyMessage(cmd string) (message p2p.Message, err error) {
	// Filter messages through open port message filter
	if err = h.base.FilterMessage(cmd); err != nil {
		return message, err
	}

	switch cmd {
	case p2p.CmdGetBlocks:
		message = &msg.GetBlocks{}

	case p2p.CmdInv:
		message = &v0.Inv{}

	case p2p.CmdGetData:
		message = &v0.GetData{}

	case p2p.CmdBlock:
		message = msg.NewBlock(&core.BlockConfirm{})

	case p2p.CmdTx:
		message = msg.NewTx(&core.Transaction{})

	case p2p.CmdNotFound:
		message = &v0.NotFound{}

	default:
		message, err = h.base.MakeEmptyMessage(cmd)
	}

	return message, err
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *HandlerV0) HandleMessage(message p2p.Message) {
	switch message := message.(type) {
	case *msg.GetBlocks:
		h.onGetBlocks(message)

	case *v0.Inv:
		h.onInv(message)

	case *v0.GetData:
		h.onGetData(message)

	case *msg.Block:
		blockConfirm, ok := message.Serializable.(*core.BlockConfirm)
		if !ok {
			return
		}
		if blockConfirm.BlockFlag {
			h.onBlock(blockConfirm)
			return
		}
		if blockConfirm.ConfirmFlag {
			h.onConfirm(blockConfirm.Confirm)
		}

	case *msg.Tx:
		h.onTx(message)

	case *v0.NotFound:
		h.onNotFound(message)

	default:
		h.base.HandleMessage(message)
	}
}

func (h *HandlerV0) onGetBlocks(req *msg.GetBlocks) error {
	node := h.base.node
	LocalNode.AcqSyncBlkReqSem()
	defer LocalNode.RelSyncBlkReqSem()

	start := chain.DefaultLedger.Blockchain.LatestLocatorHash(req.Locator)
	hashes, err := GetBlockHashes(*start, req.HashStop, p2p.MaxHeaderHashes)
	if err != nil {
		return err
	}

	if len(hashes) > 0 {
		node.SendMessage(v0.NewInv(hashes))
	}
	return nil
}

func (h *HandlerV0) onInv(inv *v0.Inv) error {
	node := h.base.node
	log.Debugf("[OnInv] count %d hashes: %v", len(inv.Hashes), inv.Hashes)

	if node.IsExternal() {
		return fmt.Errorf("receive inv message from external node")
	}

	if LocalNode.IsSyncHeaders() && !node.IsSyncHeaders() {
		return nil
	}

	for i, hash := range inv.Hashes {
		// Request block
		if !chain.DefaultLedger.BlockInLedger(*hash) &&
			(!chain.DefaultLedger.Blockchain.IsKnownOrphan(hash) || !LocalNode.IsRequestedBlock(*hash)) {

			LocalNode.AddRequestedBlock(*hash)
			node.SendMessage(v0.NewGetData(*hash))
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
	node := h.base.node
	hash := req.Hash

	block, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		log.Debugf("Can't get block from hash %s, send not found message", hash)
		node.SendMessage(v0.NewNotFound(hash))
		return err
	}

	confirm, ok := LocalNode.GetConfirm(hash)
	if !ok {
		log.Debugf("Can't get confirm from hash %s", hash)
		node.SendMessage(msg.NewBlock(&core.BlockConfirm{
			BlockFlag: true,
			Block:     block,
		}))
		return nil
	}

	node.SendMessage(msg.NewBlock(&core.BlockConfirm{
		BlockFlag:   true,
		Block:       block,
		ConfirmFlag: true,
		Confirm:     confirm,
	}))

	return nil
}

func (h *HandlerV0) onBlock(msgBlock *core.BlockConfirm) error {
	node := h.base.node
	hash := msgBlock.Block.Hash()
	log.Debug("[onblock] handlerV0 received block:", hash.String())

	if !LocalNode.IsNeighborNode(node.ID()) {
		log.Debug("Received block message from unknown peer")
		return fmt.Errorf("received block message from unknown peer")
	}

	if chain.DefaultLedger.BlockInLedger(hash) {
		h.duplicateBlocks++
		log.Debug("Receive ", h.duplicateBlocks, " duplicated block.")
		return fmt.Errorf("received duplicated block")
	}

	// Update sync timer
	LocalNode.syncTimer.update()
	chain.DefaultLedger.Store.RemoveHeaderListElement(hash)
	LocalNode.DeleteRequestedBlock(hash)

	ok, err := LocalNode.AppendBlock(msgBlock)
	if err != nil {
		log.Debugf("Received invalid block %s", hash.String())
		return fmt.Errorf("Receive invalid block %s, err: %s", hash.String(), err.Error())
	}
	if !ok {
		log.Debugf("Received unconfirmed block %s", hash.String())
	}

	//_, isOrphan, err := chain.DefaultLedger.Blockchain.AddBlock(block)
	//if err != nil {
	//	return fmt.Errorf("Block add failed: %s ,block hash %s ", err.Error(), hash.String())
	//}

	if !LocalNode.IsSyncHeaders() {
		// relay
		if !LocalNode.ExistedID(hash) {
			LocalNode.Relay(node, msgBlock)
			log.Debug("Relay block")
		}

		//if isOrphan && !LocalNode.IsRequestedBlock(hash) {
		//	orphanRoot := chain.DefaultLedger.Blockchain.GetOrphanRoot(&hash)
		//	locator, _ := chain.DefaultLedger.Blockchain.LatestBlockLocator()
		//	SendGetBlocks(node, locator, *orphanRoot)
		//}
	}

	return nil
}

func (h *HandlerV0) onConfirm(confirm *core.DPosProposalVoteSlot) error {
	err := LocalNode.AppendConfirm(confirm)
	if err != nil {
		return fmt.Errorf("receive invalid confirm %s, err: %s", confirm.Hash, err.Error())
	}

	return nil
}

func (h *HandlerV0) onTx(msgTx *msg.Tx) error {
	node := h.base.node
	tx := msgTx.Serializable.(*core.Transaction)

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
