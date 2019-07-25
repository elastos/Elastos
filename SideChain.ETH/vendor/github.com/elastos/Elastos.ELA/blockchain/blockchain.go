package blockchain

import (
	"bytes"
	"container/list"
	"errors"
	"fmt"
	"math/big"
	"sort"
	"sync"
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/events"
)

const (
	maxOrphanBlocks  = 10000
	minMemoryNodes   = 20160
	maxBlockLocators = 500
	medianTimeBlocks = 11

	// irreversibleHeight defines the max height that the chain be reorganized
	irreversibleHeight = 6
)

var (
	oneLsh256 = new(big.Int).Lsh(big.NewInt(1), 256)
)

type BlockChain struct {
	chainParams *config.Params
	db          IChainStore
	state       *state.State
	GenesisHash Uint256

	// The following fields are calculated based upon the provided chain
	// parameters.  They are also set when the instance is created and
	// can't be changed afterwards, so there is no need to protect them with
	// a separate mutex.
	minRetargetTimespan int64  // target timespan / adjustment factor
	maxRetargetTimespan int64  // target timespan * adjustment factor
	blocksPerRetarget   uint32 // target timespan / target time per block

	BestChain *BlockNode
	Root      *BlockNode
	Index     map[Uint256]*BlockNode
	IndexLock sync.RWMutex
	DepNodes  map[Uint256][]*BlockNode

	orphanLock     sync.RWMutex
	orphans        map[Uint256]*OrphanBlock
	prevOrphans    map[Uint256][]*OrphanBlock
	oldestOrphan   *OrphanBlock
	orphanConfirms map[Uint256]*payload.Confirm

	blockCache     map[Uint256]*Block
	confirmCache   map[Uint256]*payload.Confirm
	TimeSource     MedianTimeSource
	MedianTimePast time.Time
	mutex          sync.RWMutex
}

func New(db IChainStore, chainParams *config.Params, state *state.State) (*BlockChain, error) {

	targetTimespan := int64(chainParams.TargetTimespan / time.Second)
	targetTimePerBlock := int64(chainParams.TargetTimePerBlock / time.Second)
	adjustmentFactor := chainParams.AdjustmentFactor
	chain := BlockChain{
		chainParams:         chainParams,
		db:                  db,
		state:               state,
		GenesisHash:         chainParams.GenesisBlock.Hash(),
		minRetargetTimespan: targetTimespan / adjustmentFactor,
		maxRetargetTimespan: targetTimespan * adjustmentFactor,
		blocksPerRetarget:   uint32(targetTimespan / targetTimePerBlock),
		Root:                nil,
		BestChain:           nil,
		Index:               make(map[Uint256]*BlockNode),
		DepNodes:            make(map[Uint256][]*BlockNode),
		oldestOrphan:        nil,
		orphans:             make(map[Uint256]*OrphanBlock),
		prevOrphans:         make(map[Uint256][]*OrphanBlock),
		blockCache:          make(map[Uint256]*Block),
		confirmCache:        make(map[Uint256]*payload.Confirm),
		orphanConfirms:      make(map[Uint256]*payload.Confirm),
		TimeSource:          NewMedianTime(),
	}

	endHeight := chain.db.GetHeight()
	startHeight := uint32(0)
	if endHeight > minMemoryNodes {
		startHeight = endHeight - minMemoryNodes
	}

	for start := startHeight; start <= endHeight; start++ {
		hash, err := chain.db.GetBlockHash(start)
		if err != nil {
			return nil, err
		}
		header, err := chain.db.GetHeader(hash)
		if err != nil {
			return nil, err
		}
		node, err := chain.LoadBlockNode(header, &hash)
		if err != nil {
			return nil, err
		}

		// This node is now the end of the best chain.
		chain.BestChain = node
	}

	return &chain, nil
}

// InitProducerState go through all blocks since the start of DPOS
// consensus to initialize producers and votes state.
func (b *BlockChain) InitProducerState(interrupt <-chan struct{},
	start func(total uint32), increase func()) (err error) {
	bestHeight := b.db.GetHeight()
	log.Info("current block height ->", bestHeight)
	arbiters := DefaultLedger.Arbitrators
	done := make(chan struct{})
	go func() {
		// Notify initialize process start.
		startHeight := b.chainParams.VoteStartHeight
		if height, err := arbiters.RecoverFromCheckPoints(
			bestHeight); err != nil {
			log.Warn("recover form check points fail: ", err)
		} else {
			startHeight = height + 1
		}

		log.Info("[RecoverFromCheckPoints] recover start height: ", startHeight)
		if start != nil && bestHeight >= startHeight {
			start(bestHeight - startHeight)
		}
		for i := startHeight; i <= bestHeight; i++ {
			hash, e := b.db.GetBlockHash(i)
			if e != nil {
				err = e
				break
			}
			block, e := b.db.GetBlock(hash)
			if e != nil {
				err = e
				break
			}

			if block.Height >= bestHeight-uint32(
				b.chainParams.GeneralArbiters+len(b.chainParams.CRCArbiters)) {
				b.calculateTxsFee(block)
			}

			if e = PreProcessSpecialTx(block); e != nil {
				err = e
				break
			}
			confirm, _ := b.db.GetConfirm(block.Hash())
			arbiters.ProcessBlock(block, confirm)

			// Notify process increase.
			if increase != nil {
				increase()
			}
		}
		done <- struct{}{}
	}()

	select {
	case <-done:
		arbiters.Start()
		events.Notify(events.ETDirectPeersChanged,
			arbiters.GetNeedConnectArbiters())

	case <-interrupt:
	}
	return err
}

func (b *BlockChain) calculateTxsFee(block *Block) {
	for _, tx := range block.Transactions {
		if tx.IsCoinBaseTx() {
			continue
		}
		references, err := DefaultLedger.Store.GetTxReference(tx)
		if err != nil {
			log.Error("get transaction reference failed")
			return
		}
		var outputValue Fixed64
		var inputValue Fixed64
		for _, output := range tx.Outputs {
			outputValue += output.Value
		}
		for _, reference := range references {
			inputValue += reference.Value
		}
		// set Fee and FeePerKB if check has passed
		tx.Fee = inputValue - outputValue
		buf := new(bytes.Buffer)
		tx.Serialize(buf)
		tx.FeePerKB = tx.Fee * 1000 / Fixed64(len(buf.Bytes()))
	}
}

// GetState returns the DPOS state instance that stores producers and votes
// information.
func (b *BlockChain) GetState() *state.State {
	return b.state
}

func (b *BlockChain) GetHeight() uint32 {
	return b.db.GetHeight()
}

func (b *BlockChain) ProcessBlock(block *Block, confirm *payload.Confirm) (bool, bool, error) {
	b.mutex.Lock()
	defer b.mutex.Unlock()

	return b.processBlock(block, confirm)
}

func (b *BlockChain) GetHeader(hash Uint256) (*Header, error) {
	header, err := b.db.GetHeader(hash)
	if err != nil {
		return nil, errors.New("[BlockChain], GetHeader failed.")
	}
	return header, nil
}

// Get block with block hash.
func (b *BlockChain) GetBlockByHash(hash Uint256) (*Block, error) {
	return b.db.GetBlock(hash)
}

// Get DPOS block with block hash.
func (b *BlockChain) GetDposBlockByHash(hash Uint256) (*DposBlock, error) {
	if block, _ := b.db.GetBlock(hash); block != nil {
		confirm, _ := b.db.GetConfirm(hash)
		return &DposBlock{
			Block:       block,
			HaveConfirm: confirm != nil,
			Confirm:     confirm,
		}, nil
	}

	b.orphanLock.RLock()
	defer b.orphanLock.RUnlock()

	if orphan := b.orphans[hash]; orphan != nil {
		confirm := b.orphanConfirms[hash]
		return &DposBlock{
			Block:       orphan.Block,
			HaveConfirm: confirm != nil,
			Confirm:     confirm,
		}, nil
	}

	return nil, errors.New("not found dpos block in block chain")
}

func (b *BlockChain) ContainsTransaction(hash Uint256) bool {
	//TODO: implement error catch
	_, _, err := b.db.GetTransaction(hash)
	if err != nil {
		return false
	}
	return true
}

func (b *BlockChain) CurrentBlockHash() Uint256 {
	return b.db.GetCurrentBlockHash()
}

func (b *BlockChain) ProcessIllegalBlock(payload *payload.DPOSIllegalBlocks) {
	// if received inactive when synchronizing, then return
	if payload.GetBlockHeight() > b.GetHeight() {
		log.Info("received inactive tx when synchronizing")
		return
	}
	if err := DefaultLedger.Arbitrators.ProcessSpecialTxPayload(payload,
		b.BestChain.Height); err != nil {
		log.Error("process illegal block error: ", err)
	}
}

func (b *BlockChain) ProcessInactiveArbiter(payload *payload.InactiveArbitrators) {
	// if received inactive when synchronizing, then return
	if payload.GetBlockHeight() > b.GetHeight()+1 {
		log.Info("received inactive tx when synchronizing")
		return
	}
	if err := DefaultLedger.Arbitrators.ProcessSpecialTxPayload(payload,
		b.BestChain.Height); err != nil {
		log.Error("process illegal block error: ", err)
	}
}

type OrphanBlock struct {
	Block      *Block
	Expiration time.Time
}

func (b *BlockChain) ProcessOrphans(hash *Uint256) error {
	processHashes := make([]*Uint256, 0, 10)
	processHashes = append(processHashes, hash)
	for len(processHashes) > 0 {
		processHash := processHashes[0]
		processHashes[0] = nil // Prevent GC leak.
		processHashes = processHashes[1:]

		for i := 0; i < len(b.prevOrphans[*processHash]); i++ {
			orphan := b.prevOrphans[*processHash][i]
			if orphan == nil {
				continue
			}

			orphanHash := orphan.Block.Hash()
			confirm, _ := b.GetOrphanConfirm(&orphanHash)

			//log.Debug("deal with orphan block %x", orphanHash.ToArrayReverse())
			_, err := b.maybeAcceptBlock(orphan.Block, confirm)
			if err != nil {
				return err
			}

			b.RemoveOrphanBlock(orphan)
			i--
			processHashes = append(processHashes, &orphanHash)

		}
	}
	return nil
}

func (b *BlockChain) RemoveOrphanBlock(orphan *OrphanBlock) {
	b.orphanLock.Lock()
	defer b.orphanLock.Unlock()

	orphanHash := orphan.Block.Hash()
	delete(b.orphans, orphanHash)
	delete(b.orphanConfirms, orphanHash)

	prevHash := &orphan.Block.Header.Previous
	orphans := b.prevOrphans[*prevHash]
	for i := 0; i < len(orphans); i++ {
		hash := orphans[i].Block.Hash()
		if hash.IsEqual(orphanHash) {
			copy(orphans[i:], orphans[i+1:])
			orphans[len(orphans)-1] = nil
			orphans = orphans[:len(orphans)-1]
			i--
		}
	}
	b.prevOrphans[*prevHash] = orphans

	if len(b.prevOrphans[*prevHash]) == 0 {
		delete(b.prevOrphans, *prevHash)
	}

	if b.oldestOrphan == orphan {
		b.oldestOrphan = nil
	}
}

func (b *BlockChain) AddOrphanBlock(block *Block) {
	for _, oBlock := range b.orphans {
		if time.Now().After(oBlock.Expiration) {
			b.RemoveOrphanBlock(oBlock)
			continue
		}

		if b.oldestOrphan == nil || oBlock.Expiration.Before(b.oldestOrphan.Expiration) {
			b.oldestOrphan = oBlock
		}
	}

	if len(b.orphans)+1 > maxOrphanBlocks {
		b.RemoveOrphanBlock(b.oldestOrphan)
		b.oldestOrphan = nil
	}

	b.orphanLock.Lock()
	defer b.orphanLock.Unlock()

	// Insert the block into the orphan map with an expiration time
	// 1 hour from now.
	expiration := time.Now().Add(time.Hour)
	oBlock := &OrphanBlock{
		Block:      block,
		Expiration: expiration,
	}
	b.orphans[block.Hash()] = oBlock

	// Add to previous hash lookup index for faster dependency lookups.
	prevHash := &block.Header.Previous
	b.prevOrphans[*prevHash] = append(b.prevOrphans[*prevHash], oBlock)

	return
}

func (b *BlockChain) AddOrphanConfirm(confirm *payload.Confirm) {
	b.orphanLock.Lock()
	b.orphanConfirms[confirm.Proposal.BlockHash] = confirm
	b.orphanLock.Unlock()
}

func (b *BlockChain) GetOrphanConfirm(hash *Uint256) (*payload.Confirm, bool) {
	b.orphanLock.RLock()
	confirm, ok := b.orphanConfirms[*hash]
	b.orphanLock.RUnlock()
	return confirm, ok
}

func (b *BlockChain) IsKnownOrphan(hash *Uint256) bool {
	b.orphanLock.RLock()
	defer b.orphanLock.RUnlock()

	if _, exists := b.orphans[*hash]; exists {
		return true
	}

	return false
}

func (b *BlockChain) GetOrphan(hash *Uint256) *OrphanBlock {
	b.orphanLock.RLock()
	defer b.orphanLock.RUnlock()

	orphan, _ := b.orphans[*hash]
	return orphan
}

func (b *BlockChain) GetOrphanRoot(hash *Uint256) *Uint256 {
	b.orphanLock.RLock()
	defer b.orphanLock.RUnlock()

	orphanRoot := hash
	prevHash := hash
	for {
		orphan, exists := b.orphans[*prevHash]
		if !exists {
			break
		}
		orphanRoot = prevHash
		prevHash = &orphan.Block.Header.Previous
	}

	return orphanRoot
}

type BlockNode struct {
	Hash        *Uint256
	ParentHash  *Uint256
	Height      uint32
	Version     uint32
	Bits        uint32
	Timestamp   uint32
	WorkSum     *big.Int
	InMainChain bool
	Parent      *BlockNode
	Children    []*BlockNode
}

func NewBlockNode(header *Header, hash *Uint256) *BlockNode {
	var previous, current Uint256
	copy(previous[:], header.Previous[:])
	copy(current[:], hash[:])
	node := BlockNode{
		Hash:       &current,
		ParentHash: &previous,
		Height:     header.Height,
		Version:    header.Version,
		Bits:       header.Bits,
		Timestamp:  header.Timestamp,
		WorkSum:    CalcWork(header.Bits),
	}
	return &node
}

func compactToBig(compact uint32) *big.Int {
	// Extract the mantissa, sign bit, and exponent.
	mantissa := compact & 0x007fffff
	isNegative := compact&0x00800000 != 0
	exponent := uint(compact >> 24)

	// Since the base for the exponent is 256, the exponent can be treated
	// as the number of bytes to represent the full 256-bit number.  So,
	// treat the exponent as the number of bytes and shift the mantissa
	// right or left accordingly.  This is equivalent to:
	// N = mantissa * 256^(exponent-3)
	var bn *big.Int
	if exponent <= 3 {
		mantissa >>= 8 * (3 - exponent)
		bn = big.NewInt(int64(mantissa))
	} else {
		bn = big.NewInt(int64(mantissa))
		bn.Lsh(bn, 8*(exponent-3))
	}

	// Make it negative if the sign bit is set.
	if isNegative {
		bn = bn.Neg(bn)
	}

	return bn
}

// (1 << 256) / (difficultyNum + 1)
func CalcWork(bits uint32) *big.Int {
	difficultyNum := compactToBig(bits)
	if difficultyNum.Sign() <= 0 {
		return big.NewInt(0)
	}

	//denominator := new(big.Int).Add(difficultyNum, bigOne)
	denominator := new(big.Int).Add(difficultyNum, big.NewInt(1))

	return new(big.Int).Div(oneLsh256, denominator)
}

func AddChildrenWork(node *BlockNode, work *big.Int) {
	for _, childNode := range node.Children {
		childNode.WorkSum.Add(childNode.WorkSum, work)
		AddChildrenWork(childNode, work)
	}
}

func RemoveChildNode(children []*BlockNode, node *BlockNode) []*BlockNode {
	if node == nil {
		return children
	}

	for i := 0; i < len(children); i++ {
		if (*children[i].Hash).IsEqual(*node.Hash) {
			copy(children[i:], children[i+1:])
			children[len(children)-1] = nil
			return children[:len(children)-1]
		}
	}
	return children

}

func (b *BlockChain) LoadBlockNode(blockHeader *Header, hash *Uint256) (*BlockNode, error) {

	// Create the new block node for the block and set the work.
	node := NewBlockNode(blockHeader, hash)
	node.InMainChain = true

	// Add the node to the chain.
	// There are several possibilities here:
	//  1) This node is a child of an existing block node
	//  2) This node is the parent of one or more nodes
	//  3) Neither 1 or 2 is true, and this is not the first node being
	//     added to the tree which implies it's an orphan block and
	//     therefore is an error to insert into the chain
	//  4) Neither 1 or 2 is true, but this is the first node being added
	//     to the tree, so it's the root.
	prevHash := &blockHeader.Previous
	//if parentNode, ok := b.Index[*prevHash]; ok {
	if parentNode, ok := b.LookupNodeInIndex(prevHash); ok {
		// Case 1 -- This node is a child of an existing block node.
		// Update the node's work sum with the sum of the parent node's
		// work sum and this node's work, append the node as a child of
		// the parent node and set this node's parent to the parent
		// node.
		node.WorkSum = node.WorkSum.Add(parentNode.WorkSum, node.WorkSum)
		parentNode.Children = append(parentNode.Children, node)
		node.Parent = parentNode

	} else if childNodes, ok := b.DepNodes[*hash]; ok {
		// Case 2 -- This node is the parent of one or more nodes.
		// Connect this block node to all of its children and update
		// all of the children (and their children) with the new work
		// sums.
		for _, childNode := range childNodes {
			childNode.Parent = node
			node.Children = append(node.Children, childNode)
			childNode.WorkSum.Add(childNode.WorkSum, node.WorkSum)
			AddChildrenWork(childNode, node.WorkSum)
			b.Root = node
		}

	} else {
		// Case 3 -- The node does't have a parent and is not the parent
		// of another node.  This is only acceptable for the first node
		// inserted into the chain.  Otherwise it means an arbitrary
		// orphan block is trying to be loaded which is not allowed.
		if b.Root != nil {
			str := "LoadBlockNode: attempt to insert orphan block %v"
			return nil, fmt.Errorf(str, hash)
		}

		// Case 4 -- This is the root since it's the first and only node.
		b.Root = node
	}

	// Add the new node to the indices for faster lookups.
	//b.Index[*hash] = node
	b.AddNodeToIndex(node)
	b.DepNodes[*prevHash] = append(b.DepNodes[*prevHash], node)

	return node, nil
}

func (b *BlockChain) pruneBlockNodes() error {
	if b.BestChain == nil {
		return nil
	}

	newRootNode := b.BestChain
	for i := uint32(0); i < minMemoryNodes-1 && newRootNode != nil; i++ {
		newRootNode = newRootNode.Parent
	}

	// Nothing to do if there are not enough nodes.
	if newRootNode == nil || newRootNode.Parent == nil {
		return nil
	}

	deleteNodes := list.New()
	for node := newRootNode.Parent; node != nil; node = node.Parent {
		deleteNodes.PushFront(node)
	}

	// Loop through each node to prune, unlink its children, remove it from
	// the dependency index, and remove it from the node index.
	for e := deleteNodes.Front(); e != nil; e = e.Next() {
		node := e.Value.(*BlockNode)
		err := b.removeBlockNode(node)
		if err != nil {
			return err
		}
	}

	// Set the new root node.
	b.Root = newRootNode

	return nil
}

func (b *BlockChain) removeBlockNode(node *BlockNode) error {
	if node.Parent != nil {
		return fmt.Errorf("RemoveBlockNode must be called with a "+
			" node at the front of the chain - node %v", node.Hash)
	}

	// Remove the node from the node index.
	//delete(b.Index, *node.Hash)
	b.RemoveNodeFromIndex(node)

	// Unlink all of the node's children.
	for _, child := range node.Children {
		child.Parent = nil
	}
	node.Children = nil

	// Remove the reference from the dependency index.
	prevHash := node.ParentHash
	if children, ok := b.DepNodes[*prevHash]; ok {
		// Find the node amongst the children of the
		// dependencies for the parent hash and remove it.
		b.DepNodes[*prevHash] = RemoveChildNode(children, node)

		// Remove the map entry altogether if there are no
		// longer any nodes which depend on the parent hash.
		if len(b.DepNodes[*prevHash]) == 0 {
			delete(b.DepNodes, *prevHash)
		}
	}

	return nil
}

// getPrevNodeFromBlock returns a block node for the block previous to the
// passed block (the passed block's parent).  When it is already in the memory
// block chain, it simply returns it.  Otherwise, it loads the previous block
// from the block database, creates a new block node from it, and returns it.
// The returned node will be nil if the genesis block is passed.
func (b *BlockChain) getPrevNodeFromBlock(block *Block) (*BlockNode, error) {
	// Genesis block.
	prevHash := block.Header.Previous
	if prevHash.IsEqual(EmptyHash) {
		return nil, nil
	}

	// Return the existing previous block node if it's already there.
	//if bn, ok := b.Index[*prevHash]; ok {
	if bn, ok := b.LookupNodeInIndex(&prevHash); ok {
		return bn, nil
	}

	header, err := b.GetHeader(prevHash)
	if err != nil {
		return nil, err
	}
	prevBlockNode, err := b.LoadBlockNode(header, &prevHash)
	if err != nil {
		return nil, err
	}
	return prevBlockNode, nil
}

// getPrevNodeFromNode returns a block node for the block previous to the
// passed block node (the passed block node's parent).  When the node is already
// connected to a parent, it simply returns it.  Otherwise, it loads the
// associated block from the database to obtain the previous hash and uses that
// to dynamically create a new block node and return it.  The memory block
// chain is updated accordingly.  The returned node will be nil if the genesis
// block is passed.
func (b *BlockChain) getPrevNodeFromNode(node *BlockNode) (*BlockNode, error) {
	// Return the existing previous block node if it's already there.
	if node.Parent != nil {
		return node.Parent, nil
	}

	// Genesis block.
	if node.Hash.IsEqual(b.GenesisHash) {
		return nil, nil
	}

	header, err := b.GetHeader(*node.ParentHash)
	if err != nil {
		return nil, err
	}
	prevBlockNode, err := b.LoadBlockNode(header, node.ParentHash)
	if err != nil {
		return nil, err
	}

	return prevBlockNode, nil
}

// getReorganizeNodes finds the fork point between the main chain and the passed
// node and returns a list of block nodes that would need to be detached from
// the main chain and a list of block nodes that would need to be attached to
// the fork point (which will be the end of the main chain after detaching the
// returned list of block nodes) in order to reorganize the chain such that the
// passed node is the new end of the main chain.  The lists will be empty if the
// passed node is not on a side chain.
func (b *BlockChain) getReorganizeNodes(node *BlockNode) (*list.List, *list.List) {
	// Nothing to detach or attach if there is no node.
	attachNodes := list.New()
	detachNodes := list.New()
	if node == nil {
		return detachNodes, attachNodes
	}

	// Find the fork point (if any) adding each block to the list of nodes
	// to attach to the main tree.  Push them onto the list in reverse order
	// so they are attached in the appropriate order when iterating the list
	// later.
	ancestor := node
	for ; ancestor.Parent != nil; ancestor = ancestor.Parent {
		if ancestor.InMainChain {
			break
		}
		attachNodes.PushFront(ancestor)
	}

	// TODO(davec): Use prevNodeFromNode function in case the requested
	// node is further back than the what is in memory.  This shouldn't
	// happen in the normal course of operation, but the ability to fetch
	// input transactions of arbitrary blocks will likely to be exposed at
	// some point and that could lead to an issue here.

	// Start from the end of the main chain and work backwards until the
	// common ancestor adding each block to the list of nodes to detach from
	// the main chain.
	for n := b.BestChain; n != nil && n.Parent != nil; n = n.Parent {
		if n.Hash.IsEqual(*ancestor.Hash) {
			break
		}
		detachNodes.PushBack(n)
	}

	return detachNodes, attachNodes
}

// reorganizeChain reorganizes the block chain by disconnecting the nodes in the
// detachNodes list and connecting the nodes in the attach list.  It expects
// that the lists are already in the correct order and are in sync with the
// end of the current best chain.  Specifically, nodes that are being
// disconnected must be in reverse order (think of popping them off
// the end of the chain) and nodes the are being attached must be in forwards
// order (think pushing them onto the end of the chain).
func (b *BlockChain) reorganizeChain(detachNodes, attachNodes *list.List) error {
	// Ensure all of the needed side chain blocks are in the cache.
	for e := attachNodes.Front(); e != nil; e = e.Next() {
		n := e.Value.(*BlockNode)
		if _, exists := b.blockCache[*n.Hash]; !exists {
			return fmt.Errorf("block %x is missing from the side "+
				"chain block cache", n.Hash.Bytes())
		}
	}

	// Disconnect blocks from the main chain.
	for e := detachNodes.Front(); e != nil; e = e.Next() {
		n := e.Value.(*BlockNode)
		block, err := b.db.GetBlock(*n.Hash)
		if err != nil {
			return err
		}
		confirm, _ := b.db.GetConfirm(*n.Hash)

		// roll back state about the last block before disconnect
		if block.Height-1 >= b.chainParams.VoteStartHeight {
			err = DefaultLedger.Arbitrators.RollbackTo(block.Height - 1)
			if err != nil {
				return err
			}
		}

		log.Info("disconnect block:", block.Height)
		DefaultLedger.Arbitrators.DumpInfo(block.Height - 1)

		err = b.disconnectBlock(n, block, confirm)
		if err != nil {
			return err
		}
	}

	// Connect the new best chain blocks.
	for e := attachNodes.Front(); e != nil; e = e.Next() {
		n := e.Value.(*BlockNode)
		block := b.blockCache[*n.Hash]
		confirm := b.confirmCache[*n.Hash]

		log.Info("connect block:", block.Height)
		err := b.connectBlock(n, block, confirm)
		if err != nil {
			return err
		}

		// update state after connected block
		if block.Height >= b.chainParams.VoteStartHeight {
			DefaultLedger.Arbitrators.ProcessBlock(block, confirm)
			DefaultLedger.Arbitrators.DumpInfo(block.Height)
		}

		delete(b.blockCache, *n.Hash)
		delete(b.confirmCache, *n.Hash)
	}

	return nil
}

//// disconnectBlock handles disconnecting the passed node/block from the end of
//// the main (best) chain.
func (b *BlockChain) disconnectBlock(node *BlockNode, block *Block, confirm *payload.Confirm) error {
	// Make sure the node being disconnected is the end of the best chain.
	if b.BestChain == nil || !node.Hash.IsEqual(*b.BestChain.Hash) {
		return fmt.Errorf("disconnectBlock must be called with the " +
			"block at the end of the main chain")
	}

	// Remove the block from the database which houses the main chain.
	_, err := b.getPrevNodeFromNode(node)
	if err != nil {
		return err
	}

	err = b.db.RollbackBlock(*node.Hash)
	if err != nil {
		return err
	}

	// Rollback state memory DB
	if block.Height-1 >= b.chainParams.VoteStartHeight {
		err := DefaultLedger.Arbitrators.RollbackTo(block.Height - 1)
		if err != nil {
			return err
		}
	}

	// Put block in the side chain cache.
	node.InMainChain = false
	b.blockCache[*node.Hash] = block
	b.confirmCache[*node.Hash] = confirm

	//// This node's parent is now the end of the best chain.
	b.BestChain = node.Parent
	b.MedianTimePast = CalcPastMedianTime(b.BestChain)

	// Notify the caller that the block was disconnected from the main
	// chain.  The caller would typically want to react with actions such as
	// updating wallets.
	events.Notify(events.ETBlockDisconnected, block)

	return nil
}

// connectBlock handles connecting the passed node/block to the end of the main
// (best) chain.
func (b *BlockChain) connectBlock(node *BlockNode, block *Block, confirm *payload.Confirm) error {
	if err := PreProcessSpecialTx(block); err != nil {
		return err
	}

	// The block must pass all of the validation rules which depend on the
	// position of the block within the block chain.
	if err := b.checkBlockContext(block, node.Parent); err != nil {
		log.Error("PowCheckBlockContext error!", err)
		return err
	}

	if block.Height >= b.chainParams.CRCOnlyDPOSHeight {
		if err := checkBlockWithConfirmation(block, confirm); err != nil {
			return fmt.Errorf("block confirmation validate failed: %s", err)
		}
	}

	// Make sure it's extending the end of the best chain.
	prevHash := &block.Header.Previous
	if b.BestChain != nil && !prevHash.IsEqual(*b.BestChain.Hash) {
		return fmt.Errorf("connectBlock must be called with a block " +
			"that extends the main chain")
	}

	// Insert the block into the database which houses the main chain.
	if err := b.db.SaveBlock(block, confirm); err != nil {
		return err
	}

	// Add the new node to the memory main chain indices for faster
	// lookups.
	node.InMainChain = true
	//b.Index[*node.Hash] = node
	b.AddNodeToIndex(node)
	b.DepNodes[*prevHash] = append(b.DepNodes[*prevHash], node)

	// This node is now the end of the best chain.
	b.BestChain = node
	b.MedianTimePast = CalcPastMedianTime(b.BestChain)

	// Notify the caller that the block was connected to the main chain.
	// The caller would typically want to react with actions such as
	// updating wallets.
	events.Notify(events.ETBlockConnected, block)

	return nil
}

func (b *BlockChain) HaveBlock(hash *Uint256) (bool, error) {
	return b.BlockExists(hash) || b.IsKnownOrphan(hash), nil
}

func (b *BlockChain) BlockExists(hash *Uint256) bool {
	// Check memory chain first (could be main chain or side chain blocks).
	//if _, ok := b.Index[*hash]; ok {
	if _, ok := b.LookupNodeInIndex(hash); ok {
		return true
	}

	// Check in database (rest of main chain not in memory).
	return b.db.IsBlockInStore(hash)
}

func (b *BlockChain) maybeAcceptBlock(block *Block, confirm *payload.Confirm) (bool, error) {
	// Get a block node for the block previous to this one.  Will be nil
	// if this is the genesis block.
	prevNode, err := b.getPrevNodeFromBlock(block)
	if err != nil {
		log.Errorf("getPrevNodeFromBlock: %v", err)
		return false, err
	}

	// The height of this block is one more than the referenced previous
	// block.
	blockHeight := uint32(0)
	if prevNode != nil {
		blockHeight = prevNode.Height + 1
	}

	if block.Header.Height != blockHeight {
		return false, fmt.Errorf("wrong block height!")
	}

	// Prune block nodes which are no longer needed before creating
	// a new node.
	err = b.pruneBlockNodes()
	if err != nil {
		return false, err
	}

	// Create a new block node for the block and add it to the in-memory
	// block chain (could be either a side chain or the main chain).
	blockhash := block.Hash()
	newNode := NewBlockNode(&block.Header, &blockhash)
	if prevNode != nil {
		newNode.Parent = prevNode
		newNode.Height = blockHeight
		newNode.WorkSum.Add(prevNode.WorkSum, newNode.WorkSum)
	}

	// Connect the passed block to the chain while respecting proper chain
	// selection according to the chain with the most proof of work.  This
	// also handles validation of the transaction scripts.
	inMainChain, reorganized, err := b.connectBestChain(newNode, block, confirm)
	if err != nil {
		return false, err
	}

	if inMainChain && !reorganized && (block.Height >= b.chainParams.VoteStartHeight ||
		// In case of VoteStartHeight larger than (CRCOnlyDPOSHeight-PreConnectOffset)
		block.Height == b.chainParams.CRCOnlyDPOSHeight-b.chainParams.
			PreConnectOffset) {
		DefaultLedger.Arbitrators.ProcessBlock(block, confirm)
		DefaultLedger.Arbitrators.DumpInfo(block.Height)
	}

	// Notify the caller that the new block was accepted into the block
	// chain.  The caller would typically want to react by relaying the
	// inventory to other peers.
	if block.Height >= b.chainParams.CRCOnlyDPOSHeight {
		events.Notify(events.ETBlockConfirmAccepted, block)
	} else if block.Height == b.chainParams.CRCOnlyDPOSHeight-1 {
		events.Notify(events.ETNewBlockReceived, &DposBlock{
			Block:       block,
			HaveConfirm: true,
		})
		events.Notify(events.ETBlockAccepted, block)
	} else {
		events.Notify(events.ETBlockAccepted, block)
	}
	return inMainChain, nil
}

func (b *BlockChain) connectBestChain(node *BlockNode, block *Block, confirm *payload.Confirm) (bool, bool, error) {
	// We haven't selected a best chain yet or we are extending the main
	// (best) chain with a new block.  This is the most common case.

	if b.BestChain == nil || (node.Parent.Hash.IsEqual(*b.BestChain.Hash)) {
		// Perform several checks to verify the block can be connected
		// to the main chain (including whatever reorganization might
		// be necessary to get this node to the main chain) without
		// violating any rules and without actually connecting the
		// block.
		//err := s.CheckConnectBlock(node, block)
		//if err != nil {
		//	return err
		//}

		// Connect the block to the main chain.
		err := b.connectBlock(node, block, confirm)
		if err != nil {
			if err := b.state.RollbackTo(block.Height); err != nil {
				log.Error("state rollback failed: ", err)
			}
			return false, false, err
		}

		// Connect the parent node to this node.
		if node.Parent != nil {
			node.Parent.Children = append(node.Parent.Children, node)
		}

		return true, false, nil
	}

	// We're extending (or creating) a side chain which may or may not
	// become the main chain, but in either case we need the block stored
	// for future processing, so add the block to the side chain holding
	// cache.
	log.Debugf("Adding block %x to side chain cache", node.Hash.Bytes())
	b.blockCache[*node.Hash] = block
	b.confirmCache[*node.Hash] = confirm
	//b.Index[*node.Hash] = node
	b.AddNodeToIndex(node)

	// Connect the parent node to this node.
	node.InMainChain = false
	node.Parent.Children = append(node.Parent.Children, node)

	// We're extending (or creating) a side chain, but the cumulative
	// work for this new side chain is not enough to make it the new chain.
	if node.WorkSum.Cmp(b.BestChain.WorkSum) <= 0 {

		// Find the fork point.
		fork := node
		for ; fork.Parent != nil; fork = fork.Parent {
			if fork.InMainChain {
				break
			}
		}

		// Log information about how the block is forking the chain.
		if fork.Hash.IsEqual(*node.Parent.Hash) {
			log.Infof("FORK: Block %x forks the chain at height %d"+
				"/block %x, but does not cause a reorganize",
				node.Hash.Bytes(), fork.Height, fork.Hash.Bytes())
		} else {
			log.Infof("EXTEND FORK: Block %x extends a side chain "+
				"which forks the chain at height %d/block %x",
				node.Hash.Bytes(), fork.Height, fork.Hash.Bytes())
		}

		return false, false, nil
	}

	// We're extending (or creating) a side chain and the cumulative work
	// for this new side chain is more than the old best chain, so this side
	// chain needs to become the main chain.  In order to accomplish that,
	// find the common ancestor of both sides of the fork, disconnect the
	// blocks that form the (now) old fork from the main chain, and attach
	// the blocks that form the new chain to the main chain starting at the
	// common ancenstor (the point where the chain forked).
	detachNodes, attachNodes := b.getReorganizeNodes(node)
	// forbid reorganize if detaching nodes more than irreversibleHeight
	if block.Height > b.chainParams.CRCOnlyDPOSHeight &&
		detachNodes.Len() > irreversibleHeight {
		return false, false, nil
	}
	//for e := detachNodes.Front(); e != nil; e = e.Next() {
	//	n := e.Value.(*BlockNode)
	//	fmt.Println("detach", n.Hash)
	//}

	//for e := attachNodes.Front(); e != nil; e = e.Next() {
	//	n := e.Value.(*BlockNode)
	//	fmt.Println("attach", n.Hash)
	//}

	// Reorganize the chain.
	log.Infof("REORGANIZE: Block %v is causing a reorganize.", node.Hash)
	err := b.reorganizeChain(detachNodes, attachNodes)
	if err != nil {
		return false, false, err
	}

	return true, true, nil
}

// ReorganizeChain reorganize chain by specify a block, this method shall not
// be called normally because it can cause reorganizing without node work sum
// checking
func (b *BlockChain) ReorganizeChain(block *Block) error {
	hash := block.Hash()
	node, ok := b.LookupNodeInIndex(&hash)
	if !ok {
		return errors.New("node of the reorganizing block does not exist")
	}

	detachNodes, attachNodes := b.getReorganizeNodes(node)
	// forbid reorganize if detaching nodes more than irreversibleHeight
	if block.Height > b.chainParams.CRCOnlyDPOSHeight &&
		detachNodes.Len() > irreversibleHeight {
		return nil
	}

	log.Info("[ReorganizeChain] begin reorganize chain")
	err := b.reorganizeChain(detachNodes, attachNodes)
	if err != nil {
		return err
	}

	return nil
}

//(bool, bool, error)
//1. inMainChain
//2. isOphan
//3. error
func (b *BlockChain) processBlock(block *Block, confirm *payload.Confirm) (bool, bool, error) {
	blockHash := block.Hash()
	log.Debugf("[ProcessBLock] height = %d, hash = %x", block.Header.Height, blockHash.Bytes())

	// The block must not already exist in the main chain or side chains.
	exists := b.BlockExists(&blockHash)
	if exists {
		str := fmt.Sprintf("already have block %x\n", blockHash.Bytes())
		return false, false, fmt.Errorf(str)
	}

	// The block must not already exist as an orphan.
	if _, exists := b.orphans[blockHash]; exists {
		log.Debugf("already have block (orphan) %v", blockHash)
		return false, true, nil
	}

	log.Debugf("[ProcessBLock] orphan already exist= %v", exists)

	// Perform preliminary sanity checks on the block and its transactions.
	//err = PowCheckBlockSanity(block, PowLimit, b.TimeSource)
	err := b.CheckBlockSanity(block)
	if err != nil {
		log.Errorf("PowCheckBlockSanity error %s", err.Error())
		return false, false, err
	}

	blockHeader := block.Header

	// Handle orphan blocks.
	prevHash := blockHeader.Previous
	if !prevHash.IsEqual(EmptyHash) && !b.BlockExists(&prevHash) {
		log.Debugf("Adding orphan block %x with parent %x", blockHash.Bytes(), prevHash.Bytes())
		b.AddOrphanBlock(block)

		return false, true, nil
	}

	// The block has passed all context independent checks and appears sane
	// enough to potentially accept it into the block chain.
	inMainChain, err := b.maybeAcceptBlock(block, confirm)
	if err != nil {
		return false, true, err
	}

	// Accept any orphan blocks that depend on this block (they are
	// no longer orphans) and repeat for those accepted blocks until
	// there are no more.
	err = b.ProcessOrphans(&blockHash)
	if err != nil {
		//TODO inMainChain or not
		return false, false, err
	}

	//log.Debugf("Accepted block %v", blockHash)

	return inMainChain, false, nil
}

func (b *BlockChain) LatestBlockLocator() ([]*Uint256, error) {
	if b.BestChain == nil {
		// Get the latest block hash for the main chain from the
		// database.

		// Get Current Block
		blockHash := b.db.GetCurrentBlockHash()
		return b.BlockLocatorFromHash(&blockHash), nil
	}

	// The best chain is set, so use its hash.
	return b.BlockLocatorFromHash(b.BestChain.Hash), nil
}

func (b *BlockChain) AddNodeToIndex(node *BlockNode) {
	b.IndexLock.Lock()
	defer b.IndexLock.Unlock()

	b.Index[*node.Hash] = node
}

func (b *BlockChain) RemoveNodeFromIndex(node *BlockNode) {
	b.IndexLock.Lock()
	defer b.IndexLock.Unlock()
	delete(b.Index, *node.Hash)
}

func (b *BlockChain) LookupNodeInIndex(hash *Uint256) (*BlockNode, bool) {
	b.IndexLock.Lock()
	defer b.IndexLock.Unlock()
	node, exist := b.Index[*hash]

	return node, exist
}

func (b *BlockChain) BlockLocatorFromHash(inhash *Uint256) []*Uint256 {
	// The locator contains the requested hash at the very least.
	var hash Uint256
	copy(hash[:], inhash[:])
	//locator := make(Locator, 0, MaxBlockLocatorsPerMsg)
	locator := make([]*Uint256, 0)
	locator = append(locator, &hash)

	// Nothing more to do if a locator for the genesis hash was requested.
	if hash.IsEqual(b.GenesisHash) {
		return locator
	}

	// Attempt to find the height of the block that corresponds to the
	// passed hash, and if it's on a side chain, also find the height at
	// which it forks from the main chain.
	blockHeight := int32(-1)
	//node, exists := b.Index[*hash]
	node, exists := b.LookupNodeInIndex(&hash)
	if !exists {
		// Try to look up the height for passed block hash.  Assume an
		// error means it doesn't exist and just return the locator for
		// the block itself.

		block, err := b.db.GetBlock(hash)
		if err != nil {
			return locator
		}
		blockHeight = int32(block.Header.Height)
	} else {
		blockHeight = int32(node.Height)
	}

	// Generate the block locators according to the algorithm described in
	// in the Locator comment and make sure to leave room for the
	// final genesis hash.
	increment := int32(1)
	for len(locator) < maxBlockLocators-1 {
		// Once there are 10 locators, exponentially increase the
		// distance between each block locator.
		if len(locator) > 10 {
			increment *= 2
		}
		blockHeight -= increment
		if blockHeight < 1 {
			break
		}

		// The desired block height is in the main chain, so look it up
		// from the main chain database.

		h, err := b.db.GetBlockHash(uint32(blockHeight))
		if err != nil {
			log.Debugf("Lookup of known valid height failed %v", blockHeight)
			continue
		}

		locator = append(locator, &h)
	}

	// Append the appropriate genesis block.
	locator = append(locator, &b.GenesisHash)

	return locator
}

func (b *BlockChain) locateStartBlock(locator []*Uint256) *Uint256 {
	var startHash Uint256
	for _, hash := range locator {
		_, err := b.db.GetBlock(*hash)
		if err == nil {
			startHash = *hash
			break
		}
	}
	return &startHash
}

func (b *BlockChain) locateBlocks(startHash *Uint256, stopHash *Uint256, maxBlockHashes uint32) ([]*Uint256, error) {
	var count = uint32(0)
	var startHeight uint32
	var stopHeight uint32
	curHeight := b.db.GetHeight()
	if stopHash.IsEqual(EmptyHash) {
		if startHash.IsEqual(EmptyHash) {
			if curHeight > maxBlockHashes {
				count = maxBlockHashes
			} else {
				count = curHeight
			}
		} else {
			startHeader, err := b.db.GetHeader(*startHash)
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
		stopHeader, err := b.db.GetHeader(*stopHash)
		if err != nil {
			return nil, err
		}
		stopHeight = stopHeader.Height
		if !startHash.IsEqual(EmptyHash) {
			startHeader, err := b.db.GetHeader(*startHash)
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

	hashes := make([]*Uint256, 0)
	for i := uint32(1); i <= count; i++ {
		hash, err := b.db.GetBlockHash(startHeight + i)
		if err != nil {
			return nil, err
		}
		hashes = append(hashes, &hash)
	}

	return hashes, nil
}

// LocateBlocks returns the hashes of the blocks after the first known block in
// the locator until the provided stop hash is reached, or up to the provided
// max number of block hashes.
//
// In addition, there are two special cases:
//
// - When no locators are provided, the stop hash is treated as a request for
//   that block, so it will either return the stop hash itself if it is known,
//   or nil if it is unknown
// - When locators are provided, but none of them are known, hashes starting
//   after the genesis block will be returned
//
// This function is safe for concurrent access.
func (b *BlockChain) LocateBlocks(locator []*Uint256, hashStop *Uint256, maxHashes uint32) []*Uint256 {
	startHash := b.locateStartBlock(locator)
	blocks, err := b.locateBlocks(startHash, hashStop, maxHashes)
	if err != nil {
		log.Errorf("LocateBlocks error %s", err)
	}
	return blocks
}

func (b *BlockChain) MedianAdjustedTime() time.Time {
	newTimestamp := b.TimeSource.AdjustedTime()
	minTimestamp := b.MedianTimePast.Add(time.Second)

	if newTimestamp.Before(minTimestamp) {
		newTimestamp = minTimestamp
	}

	return newTimestamp
}

type timeSorter []int64

func (s timeSorter) Len() int {
	return len(s)
}

func (s timeSorter) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func (s timeSorter) Less(i, j int) bool {
	return s[i] < s[j]
}

func CalcPastMedianTime(node *BlockNode) time.Time {
	timestamps := make([]int64, medianTimeBlocks)
	numNodes := 0
	iterNode := node
	for i := 0; i < medianTimeBlocks && iterNode != nil; i++ {
		timestamps[i] = int64(iterNode.Timestamp)
		numNodes++

		iterNode = iterNode.Parent
	}

	timestamps = timestamps[:numNodes]
	sort.Sort(timeSorter(timestamps))

	medianTimestamp := timestamps[numNodes/2]
	return time.Unix(medianTimestamp, 0)
}
