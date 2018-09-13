package node

import (
	"fmt"
	"strconv"
	"time"

	chain "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/bloom"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.SideChain/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type MsgHandlerV1 struct {
	node         protocol.Noder
	continueHash *common.Uint256
}

// When something wrong on read or decode message
// this method will callback the error
func (h *MsgHandlerV1) OnError(err error) {
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
func (h *MsgHandlerV1) OnMakeMessage(cmd string) (message p2p.Message, err error) {
	// Update node last active time
	h.node.UpdateLastActive()

	switch cmd {
	case p2p.CmdVersion:
		message = new(msg.Version)
	case p2p.CmdVerAck:
		message = new(msg.VerAck)
	case p2p.CmdGetAddr:
		message = new(msg.GetAddr)
	case p2p.CmdAddr:
		message = new(msg.Addr)
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
	case p2p.CmdBlock:
		message = msg.NewBlock(new(core.Block))
	case p2p.CmdTx:
		message = msg.NewTx(new(core.Transaction))
	case p2p.CmdNotFound:
		message = new(msg.NotFound)
	case p2p.CmdMemPool:
		message = new(msg.MemPool)
	case p2p.CmdReject:
		message = new(msg.Reject)
	default:
		err = fmt.Errorf("unknown message type")
	}

	return message, err
}

// After message has been successful decoded, this method
// will be called to pass the decoded message instance
func (h *MsgHandlerV1) OnMessageDecoded(message p2p.Message) {
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
	case *msg.Block:
		err = h.onBlock(message)
	case *msg.Tx:
		err = h.onTx(message)
	case *msg.NotFound:
		err = h.onNotFound(message)
	case *msg.MemPool:
		err = h.onMemPool(message)
	case *msg.Reject:
		err = h.onReject(message)
	default:
		err = fmt.Errorf("unknown message type")
	}
	if err != nil {
		log.Error("Handler message error: " + err.Error())
	}
}

func (h *MsgHandlerV1) onVersion(version *msg.Version) error {
	node := h.node

	// Exclude the node itself
	if version.Nonce == LocalNode.ID() {
		log.Warn("The node handshake with itself")
		node.CloseConn()
		return fmt.Errorf("The node handshake with itself")
	}

	if node.State() != p2p.INIT && node.State() != p2p.HAND {
		log.Warn("Unknown status to receive version")
		return fmt.Errorf("Unknown status to receive version")
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

	ip, _ := node.Addr16()
	addr := p2p.NetAddress{
		Time:     node.GetTime(),
		Services: version.Services,
		IP:       ip,
		Port:     version.Port,
		ID:       version.Nonce,
	}
	LocalNode.AddAddressToKnownAddress(addr)

	var message p2p.Message
	if node.State() == p2p.INIT {
		node.SetState(p2p.HANDSHAKE)
		message = NewVersion(LocalNode)
	} else if node.State() == p2p.HAND {
		node.SetState(p2p.HANDSHAKED)
		message = new(msg.VerAck)
	}
	node.Send(message)

	return nil
}

func (h *MsgHandlerV1) onVerAck(verAck *msg.VerAck) error {
	node := h.node
	if node.State() != p2p.HANDSHAKE && node.State() != p2p.HANDSHAKED {
		log.Warn("unknown status to received verack")
		return fmt.Errorf("unknown status to received verack")
	}

	if node.State() == p2p.HANDSHAKE {
		node.Send(verAck)
	}

	node.SetState(p2p.ESTABLISH)
	go node.Heartbeat()

	if LocalNode.NeedMoreAddresses() {
		node.Send(new(msg.Addr))
	}
	addr := node.Addr()
	port := node.Port()
	nodeAddr := addr + ":" + strconv.Itoa(int(port))
	LocalNode.RemoveFromHandshakeQueue(node)
	LocalNode.RemoveFromConnectingList(nodeAddr)
	return nil
}

func (h *MsgHandlerV1) onAddrsReq(req *msg.GetAddr) error {
	addrs := LocalNode.RandSelectAddresses()
	h.node.Send(msg.NewAddr(addrs))
	return nil
}

func (h *MsgHandlerV1) onAddrs(addrs *msg.Addr) error {
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

func (h *MsgHandlerV1) onFilterLoad(msg *msg.FilterLoad) error {
	// Only allow filterload requests if server enabled OpenService
	if LocalNode.Services()&protocol.OpenService != protocol.OpenService {
		h.node.CloseConn()
		return fmt.Errorf("peer %d sent filterload request with open service disabled", h.node.ID())
	}

	h.node.LoadFilter(msg)
	return nil
}

func (h *MsgHandlerV1) onPing(ping *msg.Ping) error {
	h.node.SetHeight(ping.Nonce)
	h.node.Send(msg.NewPong(chain.DefaultLedger.Blockchain.BestChain.Height))
	return nil
}

func (h *MsgHandlerV1) onPong(pong *msg.Pong) error {
	h.node.SetHeight(pong.Nonce)
	return nil
}

func (h *MsgHandlerV1) onGetBlocks(req *msg.GetBlocks) error {
	node := h.node
	LocalNode.AcqSyncHdrReqSem()
	defer LocalNode.RelSyncHdrReqSem()

	start := chain.DefaultLedger.Blockchain.LatestLocatorHash(req.Locator)
	hashes, err := GetBlockHashes(*start, req.HashStop, p2p.MaxBlocksPerMsg)
	if err != nil {
		return err
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
		node.Send(inv)
	}

	return nil
}

func (h *MsgHandlerV1) onInventory(inv *msg.Inventory) error {
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
		hash := iv.Hash
		switch iv.Type {
		case msg.InvTypeBlock:
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
			if _, ok := LocalNode.GetTxInPool(hash); !ok {
				getData.AddInvVect(iv)
			}
		default:
			continue
		}
	}

	if len(getData.InvList) > 0 {
		node.Send(getData)
	}
	return nil
}

func (h *MsgHandlerV1) onGetData(getData *msg.GetData) error {
	node := h.node
	notFound := msg.NewNotFound()

	for _, iv := range getData.InvList {
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

			if h.continueHash != nil && h.continueHash.IsEqual(iv.Hash) {
				best := chain.DefaultLedger.Blockchain.BestChain
				inv := msg.NewInventory()
				inv.AddInvVect(msg.NewInvVect(msg.InvTypeBlock, best.Hash))
				node.Send(inv)
				h.continueHash = nil
			}

		case msg.InvTypeTx:
			tx, ok := LocalNode.GetTxInPool(iv.Hash)
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

func (h *MsgHandlerV1) onBlock(msgBlock *msg.Block) error {
	node := h.node
	block := msgBlock.Block.(*core.Block)

	hash := block.Hash()
	if !LocalNode.IsNeighborNoder(node) {
		return fmt.Errorf("received block message from unknown peer")
	}

	if chain.DefaultLedger.BlockInLedger(hash) {
		log.Trace("Receive duplicated block, ", hash.String())
		return nil
	}

	// Update sync timer
	LocalNode.syncTimer.update()
	chain.DefaultLedger.Store.RemoveHeaderListElement(hash)
	LocalNode.DeleteRequestedBlock(hash)

	_, isOrphan, err := chain.DefaultLedger.Blockchain.AddBlock(block)
	if err != nil {
		reject := msg.NewReject(msgBlock.CMD(), msg.RejectInvalid, err.Error())
		reject.Hash = block.Hash()

		node.Send(reject)
		return fmt.Errorf("Block add failed: %s ,block hash %s ", err.Error(), hash.String())
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

	return nil
}

func (h *MsgHandlerV1) onTx(msgTx *msg.Tx) error {
	node := h.node
	tx := msgTx.Transaction.(*core.Transaction)

	if !LocalNode.IsNeighborNoder(node) {
		return fmt.Errorf("received transaction message from unknown peer")
	}

	if LocalNode.IsSyncHeaders() {
		return nil
	}

	if LocalNode.ExistedID(tx.Hash()) {
		reject := msg.NewReject(msgTx.CMD(), msg.RejectDuplicate, "duplicate transaction")
		reject.Hash = tx.Hash()
		node.Send(reject)
		return fmt.Errorf("[HandlerEIP001] Transaction already exsisted")
	}

	if errCode := LocalNode.AppendToTxnPool(tx); errCode != errors.Success {
		reject := msg.NewReject(msgTx.CMD(), msg.RejectInvalid, errCode.Message())
		reject.Hash = tx.Hash()
		node.Send(reject)
		return fmt.Errorf("[HandlerEIP001] VerifyTransaction failed when AppendToTxnPool")
	}

	LocalNode.Relay(node, tx)
	log.Infof("Relay Transaction type %s hash %s", chain.TxFeeHelper.Name(tx.TxType), tx.Hash().String())
	LocalNode.IncRxTxnCnt()

	return nil
}

func (h *MsgHandlerV1) onNotFound(inv *msg.NotFound) error {
	for _, iv := range inv.InvList {
		log.Warnf("data not found type: %s hash: %s", iv.Type.String(), iv.Hash.String())
	}
	return nil
}

func (h *MsgHandlerV1) onMemPool(*msg.MemPool) error {
	// Only allow mempool requests if server enabled OpenService
	if LocalNode.Services()&protocol.OpenService != protocol.OpenService {
		h.node.CloseConn()
		return fmt.Errorf("peer %d sent mempool request with spen service disabled", h.node.ID())
	}

	txMemPool := LocalNode.GetTxsInPool()
	inv := msg.NewInventory()

	for _, tx := range txMemPool {
		if !h.node.BloomFilter().IsLoaded() || h.node.BloomFilter().MatchTxAndUpdate(tx) {
			txId := tx.Hash()
			inv.AddInvVect(msg.NewInvVect(msg.InvTypeTx, &txId))
		}
	}

	if len(inv.InvList) > 0 {
		h.node.Send(inv)
	}

	return nil
}

func (h *MsgHandlerV1) onReject(msg *msg.Reject) error {
	return fmt.Errorf("Received reject message from peer %d: Code: %s, Hash %s, Reason: %s",
		h.node.ID(), msg.Code.String(), msg.Hash.String(), msg.Reason)
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
				return nil, fmt.Errorf("do not have header to send")
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
