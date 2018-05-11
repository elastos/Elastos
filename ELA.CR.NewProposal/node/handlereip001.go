package node

import (
	"fmt"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

var DuplicateBlocks int

type HandlerEIP001 struct {
	HandlerBase
}

func NewHandlerEIP001(node protocol.Noder) *HandlerV0 {
	return &HandlerV0{HandlerBase{node}}
}

// After message header decoded, this method will be
// called to create the message instance with the CMD
// which is the message type of the received message
func (h *HandlerEIP001) OnMakeMessage(cmd string) (message p2p.Message, err error) {

	// Update node last active time
	h.node.UpdateLastActive()

	switch cmd {
	case p2p.CmdPing:
		message = new(msg.Ping)
	case p2p.CmdPong:
		message = new(msg.Pong)
	case p2p.CmdFilterLoad:
		message = new(msg.FilterLoad)
	case p2p.CmdGetBlocks:
		message = new(msg.GetBlocks)
	case p2p.CmdInv:
		message = new(msg.Inventory)
	case p2p.CmdGetData:
		message = new(msg.GetData)
	case p2p.CmdNotFound:
		message = new(msg.NotFound)
	case p2p.CmdMemPool:
		message = new(msg.MemPool)
	case p2p.CmdReject:
		message = new(msg.Reject)
	default:
		message, err = h.HandlerBase.OnMakeMessage(cmd)
	}

	return message, err
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *HandlerEIP001) OnMessageDecoded(message p2p.Message) {
	var err error
	switch message := message.(type) {
	case *msg.Ping:
		err = h.onPing(message)
	case *msg.Pong:
		err = h.onPong(message)
	case *msg.FilterLoad:
		err = h.onFilterLoad(message)
	case *msg.GetBlocks:
		err = h.onGetBlocks(message)
	case *msg.Inventory:
		err = h.onInventory(message)
	case *msg.GetData:
		err = h.onGetData(message)
	case *msg.NotFound:
		err = h.onNotFound(message)
	case *msg.MemPool:
		err = h.onMemPool(message)
	case *msg.Reject:
		err = h.onReject(message)
	default:
		h.HandlerBase.OnMessageDecoded(message)
	}
	if err != nil {
		log.Error("Handler message error: " + err.Error())
	}
}

func (h *HandlerEIP001) onFilterLoad(msg *msg.FilterLoad) error {
	log.Debug()
	if !config.Parameters.SPVService {
		return nil
	}

	h.node.LoadFilter(msg)

	return nil
}

func (h *HandlerEIP001) onPing(ping *msg.Ping) error {
	log.Debug()
	h.node.SetHeight(ping.Nonce)
	h.node.Send(msg.NewPong(chain.DefaultLedger.Store.GetHeight()))
	return nil
}

func (h *HandlerEIP001) onPong(pong *msg.Pong) error {
	log.Debug()
	h.node.SetHeight(pong.Nonce)
	return nil
}

func (h *HandlerEIP001) onGetBlocks(req *msg.GetBlocks) error {
	log.Debug()
	node := h.node
	LocalNode.AcqSyncHdrReqSem()
	defer LocalNode.RelSyncHdrReqSem()

	start := chain.DefaultLedger.Blockchain.LatestLocatorHash(req.Locator)
	hashes, err := GetBlockHashes(*start, req.HashStop)
	if err != nil {
		return err
	}

	inv := msg.NewInventory()
	for i := range hashes {
		inv.AddInvVect(msg.NewInvVect(msg.InvTypeBlock, hashes[i]))
	}

	node.Send(inv)

	return nil
}

func (h *HandlerEIP001) onInventory(inv *msg.Inventory) error {
	log.Debug()
	node := h.node

	if LocalNode.IsSyncHeaders() && !node.IsSyncHeaders() {
		return nil
	}

	// Attempt to find the final block in the inventory list.  There may
	// not be one.
	lastBlock := -1
	for i := len(inv.InvList) - 1; i >= 0; i-- {
		if inv.InvList[i].Type == msg.InvTypeBlock {
			lastBlock = i
			break
		}
	}

	getData := msg.NewGetData()

	for i, iv := range inv.InvList {
		// Ignore unsupported inventory types.
		hash := iv.Hash
		switch iv.Type {
		case msg.InvTypeBlock:
			// Block need to be request
			if !chain.DefaultLedger.BlockInLedger(hash) && !LocalNode.RequestedBlockExisted(hash) {
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
			if _, ok := node.GetTxnPool(false)[hash]; !ok {
				getData.AddInvVect(iv)
			}
		default:
			continue
		}
	}

	node.Send(getData)
	return nil
}

func (h *HandlerEIP001) onGetData(inv *msg.GetData) error {
	log.Debug()
	node := h.node
	notFound := msg.NewNotFound()

	for _, iv := range inv.InvList {
		switch iv.Type {
		case msg.InvTypeBlock:
			block, err := chain.DefaultLedger.Store.GetBlock(iv.Hash)
			if err != nil {
				log.Debug("Can't get block from hash: ", iv.Hash, " ,send not found message")
				notFound.AddInvVect(iv)
				return err
			}
			log.Debug("block height is ", block.Header.Height, " ,hash is ", iv.Hash.String())

			node.Send(msg.NewBlock(block))

		case msg.InvTypeTx:
			tx, ok := node.GetTxnPool(false)[iv.Hash]
			if !ok {
				notFound.AddInvVect(iv)
				continue
			}

			node.Send(msg.NewTx(tx))

		case msg.InvTypeFilteredBlock:
			if !h.node.BloomFilter().IsLoaded() {
				return nil
			}

			block, err := chain.DefaultLedger.Store.GetBlock(iv.Hash)
			if err != nil {
				log.Debug("Can't get block from hash: ", iv.Hash, " ,send not found message")
				notFound.AddInvVect(iv)
				return err
			}

			merkle, matchedIndexes := bloom.NewMerkleBlock(block, h.node.BloomFilter())

			// Send merkleblock
			node.Send(merkle)

			// Send any matched transactions
			for _, index := range matchedIndexes {
				node.Send(msg.NewTx(block.Transactions[index]))
			}

		default:
			log.Warnf("Unknown type in inventory request %d", iv.Type)
			continue
		}
	}

	return nil
}

func (h *HandlerEIP001) onNotFound(inv *msg.NotFound) error {
	for _, iv := range inv.InvList {
		log.Warnf("data not found type: %s hash: %s", iv.Type.String(), iv.Hash.String())
	}
	return nil
}

func (h *HandlerEIP001) onMemPool(*msg.MemPool) error {
	// Only allow mempool requests if server enabled SPV service
	if LocalNode.Services()&protocol.SPVService != protocol.SPVService {
		h.node.CloseConn()
		return fmt.Errorf("peer %d sent mempool request with SPV service disabled", h.node.ID())
	}

	txMemPool := LocalNode.GetTxnPool(false)
	inv := msg.NewInventory()

	for _, tx := range txMemPool {
		if !h.node.BloomFilter().IsLoaded() || h.node.BloomFilter().MatchTxAndUpdate(tx) {
			txId := tx.Hash()
			inv.AddInvVect(msg.NewInvVect(msg.InvTypeTx, &txId))
		}
	}

	return nil
}

func (h *HandlerEIP001) onReject(msg *msg.Reject) error {
	return nil
}
