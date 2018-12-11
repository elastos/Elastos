package node

import (
	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

var _ protocol.Handler = (*HandlerEIP001)(nil)

type HandlerEIP001 struct {
	base         HandlerBase
	continueHash *common.Uint256
}

func NewHandlerEIP001(node protocol.Noder) *HandlerEIP001 {
	return &HandlerEIP001{base: HandlerBase{node: node}}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerEIP001) MakeEmptyMessage(cmd string) (message p2p.Message, err error) {
	// Filter messages through open port message filter
	if err = h.base.FilterMessage(cmd); err != nil {
		return message, err
	}

	switch cmd {
	case p2p.CmdFilterLoad:
		message = &msg.FilterLoad{}

	case p2p.CmdGetBlocks:
		message = &msg.GetBlocks{}

	case p2p.CmdInv:
		message = &msg.Inventory{}

	case p2p.CmdGetData:
		message = &msg.GetData{}

	case p2p.CmdBlock:
		message = msg.NewBlock(&types.Block{})

	case p2p.CmdTx:
		message = msg.NewTx(&types.Transaction{})

	case p2p.CmdNotFound:
		message = &msg.NotFound{}

	case p2p.CmdMemPool:
		message = &msg.MemPool{}

	case p2p.CmdReject:
		message = &msg.Reject{}

	default:
		message, err = h.base.MakeEmptyMessage(cmd)
	}

	return message, err
}

func (h *HandlerEIP001) HandleMessage(message p2p.Message) {
	switch message := message.(type) {
	case *msg.FilterLoad:
		h.onFilterLoad(message)

	case *msg.GetBlocks:
		h.onGetBlocks(message)

	case *msg.Inventory:
		h.onInventory(message)

	case *msg.GetData:
		h.onGetData(message)

	case *msg.Block:
		h.onBlock(message)

	case *msg.Tx:
		h.onTx(message)

	case *msg.NotFound:
		h.onNotFound(message)

	case *msg.MemPool:
		h.onMemPool(message)

	case *msg.Reject:
		h.onReject(message)

	default:
		h.base.HandleMessage(message)
	}
}

func (h *HandlerEIP001) onFilterLoad(msg *msg.FilterLoad) {
	h.base.node.LoadFilter(msg)
}

func (h *HandlerEIP001) onGetBlocks(req *msg.GetBlocks) {
	node := h.base.node
	LocalNode.AcqSyncBlkReqSem()
	defer LocalNode.RelSyncBlkReqSem()

	start := chain.DefaultLedger.Blockchain.LatestLocatorHash(req.Locator)
	hashes, err := GetBlockHashes(*start, req.HashStop, p2p.MaxBlocksPerMsg)
	if err != nil {
		log.Error(err)
		return
	}

	inv := msg.NewInventory()
	for i := range hashes {
		inv.AddInvVect(msg.NewInvVect(msg.InvTypeBlock, hashes[i]))
	}

	invListLen := len(inv.InvList)
	if invListLen > 0 {
		if invListLen == p2p.MaxBlocksPerMsg {
			continueHash := inv.InvList[invListLen-1].Hash
			h.continueHash = &continueHash
		}
		node.SendMessage(inv)
	}
}

func (h *HandlerEIP001) onInventory(inv *msg.Inventory) {
	node := h.base.node
	if LocalNode.IsSyncHeaders() && !node.IsSyncHeaders() {
		return
	}

	// Attempt to find the final block in the inventory list.
	// There may not be one.
	lastBlock := -1
	for i := len(inv.InvList) - 1; i >= 0; i-- {
		if inv.InvList[i].Type == msg.InvTypeBlock {
			lastBlock = i
			break
		}
	}

	getData := msg.NewGetData()

	for i, iv := range inv.InvList {
		hash := iv.Hash
		switch iv.Type {
		case msg.InvTypeBlock:
			if node.IsExternal() {
				log.Debug("receive InvTypeBlock from external node")
				return
			}

			haveInv := chain.DefaultLedger.BlockInLedger(hash) ||
				chain.DefaultLedger.Blockchain.IsKnownOrphan(&hash) || LocalNode.IsRequestedBlock(hash)

			// Block need to be request
			if !haveInv {
				LocalNode.AddRequestedBlock(hash)
				getData.AddInvVect(iv)
				continue
			}

			// Request fork chain
			if chain.DefaultLedger.Blockchain.IsKnownOrphan(&hash) {
				orphanRoot := chain.DefaultLedger.Blockchain.GetOrphanRoot(&hash)
				locator, err := chain.DefaultLedger.Blockchain.LatestBlockLocator()
				if err != nil {
					log.Errorf(" Failed to get block locator for the latest block: %v", err)
					continue
				}
				SendGetBlocks(node, locator, *orphanRoot)
				continue
			}

			// Request next hashes
			if i == lastBlock {
				locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)
				SendGetBlocks(node, locator, common.EmptyHash)
			}
		case msg.InvTypeTx:
			if _, ok := LocalNode.GetTransactionPool(false)[hash]; !ok {
				getData.AddInvVect(iv)
			}
		default:
			continue
		}
	}

	node.SendMessage(getData)
}

func (h *HandlerEIP001) onGetData(getData *msg.GetData) {
	node := h.base.node
	notFound := msg.NewNotFound()

	for _, iv := range getData.InvList {
		switch iv.Type {
		case msg.InvTypeBlock:
			block, err := chain.DefaultLedger.Store.GetBlock(iv.Hash)
			if err != nil {
				log.Debug("Can't get block from hash: ", iv.Hash, " ,send not found message")
				notFound.AddInvVect(iv)
				continue
			}
			log.Debug("block height is ", block.Header.Height, " ,hash is ", iv.Hash.String())

			node.SendMessage(msg.NewBlock(block))

			if h.continueHash != nil && h.continueHash.IsEqual(iv.Hash) {
				best := chain.DefaultLedger.Blockchain.BestChain
				inv := msg.NewInventory()
				inv.AddInvVect(msg.NewInvVect(msg.InvTypeBlock, best.Hash))
				node.SendMessage(inv)
				h.continueHash = nil
			}

		case msg.InvTypeTx:
			tx, ok := LocalNode.GetTransactionPool(false)[iv.Hash]
			if !ok {
				notFound.AddInvVect(iv)
				continue
			}

			node.SendMessage(msg.NewTx(tx))

		case msg.InvTypeFilteredBlock:
			if !node.BloomFilter().IsLoaded() {
				return
			}

			block, err := chain.DefaultLedger.Store.GetBlock(iv.Hash)
			if err != nil {
				log.Debug("Can't get block from hash: ", iv.Hash, " ,send not found message")
				notFound.AddInvVect(iv)
				continue
			}

			merkle, matchedIndexes := bloom.NewMerkleBlock(block, node.BloomFilter())

			// Send merkleblock
			node.SendMessage(merkle)

			// Send any matched transactions
			for _, index := range matchedIndexes {
				node.SendMessage(msg.NewTx(block.Transactions[index]))
			}

		default:
			log.Warnf("Unknown type in inventory request %d", iv.Type)
			continue
		}
	}

	if len(notFound.InvList) > 0 {
		node.SendMessage(notFound)
	}
}

func (h *HandlerEIP001) onBlock(msgBlock *msg.Block) {
	node := h.base.node
	block := msgBlock.Serializable.(*types.Block)

	hash := block.Hash()
	if !LocalNode.IsNeighborNode(node.ID()) {
		log.Warn("receive block message from unknown peer")
		node.Disconnect()
		return
	}

	if chain.DefaultLedger.BlockInLedger(hash) {
		log.Debugf("receive duplicated block %s", hash.String())
		return
	}

	// Update sync timer
	LocalNode.syncTimer.update()
	LocalNode.DeleteRequestedBlock(hash)

	_, isOrphan, err := chain.DefaultLedger.Blockchain.AddBlock(block)
	if err != nil {
		reject := msg.NewReject(msgBlock.CMD(), msg.RejectInvalid, err.Error())
		reject.Hash = block.Hash()

		node.SendMessage(reject)
		log.Warnf("Block add failed: %s ,block hash %s ", err.Error(), hash.String())
		return
	}

	if isOrphan {
		orphanRoot := chain.DefaultLedger.Blockchain.GetOrphanRoot(&hash)
		locator, _ := chain.DefaultLedger.Blockchain.LatestBlockLocator()
		SendGetBlocks(node, locator, *orphanRoot)
	}

	if !LocalNode.IsSyncHeaders() && !LocalNode.ExistedID(hash) {
		LocalNode.Relay(node, block)
		log.Debug("Relay block")
	}
}

func (h *HandlerEIP001) onTx(msgTx *msg.Tx) {
	node := h.base.node
	tx := msgTx.Serializable.(*types.Transaction)

	if !LocalNode.IsNeighborNode(node.ID()) {
		log.Warn("received transaction message from unknown peer")
		node.Disconnect()
		return
	}

	if LocalNode.IsSyncHeaders() {
		return
	}

	if LocalNode.ExistedID(tx.Hash()) {
		reject := msg.NewReject(msgTx.CMD(), msg.RejectDuplicate, "duplicate transaction")
		reject.Hash = tx.Hash()
		node.SendMessage(reject)
		log.Debug("[HandlerEIP001] Transaction already exsisted")
		return
	}

	if errCode := LocalNode.AppendToTxnPool(tx); errCode != errors.Success {
		reject := msg.NewReject(msgTx.CMD(), msg.RejectInvalid, errCode.Message())
		reject.Hash = tx.Hash()
		node.SendMessage(reject)
		log.Debug("[HandlerEIP001] VerifyTransaction failed when AppendToTxnPool")
		return
	}

	LocalNode.Relay(node, tx)
	log.Infof("Relay Transaction type %s hash %s", tx.TxType.Name(), tx.Hash().String())
	LocalNode.IncRxTxnCnt()
}

func (h *HandlerEIP001) onNotFound(inv *msg.NotFound) {
	for _, iv := range inv.InvList {
		log.Warnf("data not found type: %s hash: %s", iv.Type.String(), iv.Hash.String())
	}
}

func (h *HandlerEIP001) onMemPool(*msg.MemPool) {
	node := h.base.node
	// Only allow mempool requests if server enabled SPV service
	if LocalNode.Services()&protocol.OpenService != protocol.OpenService {
		log.Debugf("peer %s sent mempool request with SPV service disabled", node)
		node.Disconnect()
		return
	}

	txMemPool := LocalNode.GetTransactionPool(false)
	inv := msg.NewInventory()

	for _, tx := range txMemPool {
		if !node.BloomFilter().IsLoaded() || node.BloomFilter().MatchTxAndUpdate(tx) {
			txID := tx.Hash()
			inv.AddInvVect(msg.NewInvVect(msg.InvTypeTx, &txID))
		}
	}

	if len(inv.InvList) > 0 {
		node.SendMessage(inv)
	}
}

func (h *HandlerEIP001) onReject(msg *msg.Reject) {
	log.Debugf("Received reject message from peer %s: Code: %s, Hash %s, Reason: %s",
		h.base.node, msg.Code.String(), msg.Hash.String(), msg.Reason)
}
