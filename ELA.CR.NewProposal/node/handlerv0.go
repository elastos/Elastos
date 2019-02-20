package node

import (
	"fmt"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/msg/v0"
	"github.com/elastos/Elastos.ELA/protocol"
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
		message = msg.NewBlock(&types.DposBlock{})

	case p2p.CmdTx:
		message = msg.NewTx(&types.Transaction{})

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
		h.onBlock(message)

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
	log.Debugf("[onInv] count %d hashes: %v", len(inv.Hashes), inv.Hashes)

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
				log.Errorf("failed to get block locator for the latest block: %v", err)
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

	block, err := chain.DefaultLedger.Blockchain.GetBlock(hash)
	if err != nil {
		log.Debugf("can't get block from hash %s, send not found message", hash)
		node.SendMessage(v0.NewNotFound(hash))
		return err
	}

	var confirm *types.DPosProposalVoteSlot
	confirm, err = chain.DefaultLedger.Store.GetConfirm(hash)
	if err != nil {
		var ok bool
		confirm, ok = LocalNode.GetConfirm(hash)
		if !ok {
			log.Debugf("can't get confirm from hash %s, only send block", hash)
			node.SendMessage(msg.NewBlock(&types.DposBlock{
				BlockFlag: true,
				Block:     block,
			}))
			return nil
		}
	}

	log.Debugf("send block and confirm: %s", hash)
	node.SendMessage(msg.NewBlock(&types.DposBlock{
		BlockFlag:   true,
		Block:       block,
		ConfirmFlag: true,
		Confirm:     confirm,
	}))

	return nil
}

func (h *HandlerV0) onBlock(msg *msg.Block) error {
	var blockHash common.Uint256
	var isOrphan bool
	node := h.base.node
	dposBlock := msg.Serializable.(*types.DposBlock)
	if !dposBlock.BlockFlag && !dposBlock.ConfirmFlag {
		return fmt.Errorf("nil block confirm")
	} else if !dposBlock.BlockFlag && dposBlock.ConfirmFlag {
		log.Info("[onBlock] received confirm from main chain p2p")
		var err error
		_, isOrphan, err = LocalNode.AppendConfirm(dposBlock.Confirm)
		if err != nil {
			return fmt.Errorf("[onConfirm] receive invalid confirm %s, err: %s", dposBlock.Confirm.Hash, err.Error())
		}
		blockHash = dposBlock.Confirm.Hash
		return nil
	} else {
		blockHash = dposBlock.Block.Hash()
		log.Debug("[onBlock] handlerV0 received block:", blockHash.String())

		if !LocalNode.IsNeighborNode(node) {
			log.Debug("Received block message from unknown peer")
			return fmt.Errorf("received block message from unknown peer")
		}

		if chain.DefaultLedger.BlockInLedger(blockHash) {
			h.duplicateBlocks++
			log.Debug("receive ", h.duplicateBlocks, " duplicated block.")
			return fmt.Errorf("received duplicated block")
		}

		// Update sync timer
		LocalNode.syncTimer.update()
		LocalNode.DeleteRequestedBlock(blockHash)

		var err error
		_, isOrphan, err = chain.DefaultLedger.HeightVersions.AddDposBlock(dposBlock)
		if err != nil {
			log.Debugf("received invalid block %s, err: %s", blockHash.String(), err.Error())
			return fmt.Errorf("receive invalid block %s, err: %s", blockHash.String(), err.Error())
		}
	}

	if !LocalNode.IsSyncHeaders() {
		// relay
		if !LocalNode.ExistedID(blockHash) {
			LocalNode.Relay(node, dposBlock)
			log.Debug("relay block")
		}

		if isOrphan && !LocalNode.IsRequestedBlock(blockHash) {
			locator, _ := chain.DefaultLedger.Blockchain.LatestBlockLocator()
			SendGetBlocks(node, locator, dposBlock.Block.Hash())
		}
	}

	return nil
}

func (h *HandlerV0) onTx(msgTx *msg.Tx) error {
	node := h.base.node
	tx := msgTx.Serializable.(*types.Transaction)

	if !LocalNode.ExistedID(tx.Hash()) && !LocalNode.IsSyncHeaders() {
		if errCode := LocalNode.AppendToTxnPool(tx); errCode != errors.Success {
			return fmt.Errorf("[HandlerBase] verifyTransaction failed when AppendToTxnPool")
		}
		LocalNode.Relay(node, tx)
		log.Debugf("relay transaction hash %s type %s", tx.Hash().String(), tx.TxType.Name())
		LocalNode.IncRxTxnCnt()
	}

	return nil
}

func (h *HandlerV0) onNotFound(msg *v0.NotFound) error {
	log.Debug("received not found message, hash: ", msg.Hash.String())
	return nil
}
