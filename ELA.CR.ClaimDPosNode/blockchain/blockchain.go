package blockchain

import (
	"container/list"
	"errors"
	"encoding/binary"
	"fmt"
	"math/big"
	"math/rand"
	"sort"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/log"

	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

const (
	MaxBlockLocatorsPerMsg = 500
	medianTimeBlocks       = 11
)

var (
	maxOrphanBlocks = config.Parameters.ChainParam.MaxOrphanBlocks
	MinMemoryNodes  = config.Parameters.ChainParam.MinMemoryNodes
)

var (
	oneLsh256 = new(big.Int).Lsh(big.NewInt(1), 256)
)

type Blockchain struct {
	BlockHeight    uint32
	GenesisHash    Uint256
	BestChain      *BlockNode
	Root           *BlockNode
	Index          map[Uint256]*BlockNode
	IndexLock      sync.RWMutex
	DepNodes       map[Uint256][]*BlockNode
	Orphans        map[Uint256]*OrphanBlock
	PrevOrphans    map[Uint256][]*OrphanBlock
	OldestOrphan   *OrphanBlock
	BlockCache     map[Uint256]*Block
	TimeSource     MedianTimeSource
	MedianTimePast time.Time
	OrphanLock     sync.RWMutex
	BCEvents       *events.Event
	mutex          sync.RWMutex
	AssetID        Uint256
}

func NewBlockchain(height uint32) *Blockchain {
	return &Blockchain{
		BlockHeight:  height,
		Root:         nil,
		BestChain:    nil,
		Index:        make(map[Uint256]*BlockNode),
		DepNodes:     make(map[Uint256][]*BlockNode),
		OldestOrphan: nil,
		Orphans:      make(map[Uint256]*OrphanBlock),
		PrevOrphans:  make(map[Uint256][]*OrphanBlock),
		BlockCache:   make(map[Uint256]*Block),
		TimeSource:   NewMedianTime(),

		BCEvents: events.NewEvent(),
		AssetID:  EmptyHash,
	}
}

func Init(store IChainStore) error {
	genesisBlock, err := GetGenesisBlock()
	if err != nil {
		return errors.New("[Blockchain], NewBlockchainWithGenesisBlock failed.")
	}

	DefaultLedger = new(Ledger)
	DefaultLedger.Blockchain = NewBlockchain(0)
	DefaultLedger.Store = store
	DefaultLedger.Blockchain.AssetID = genesisBlock.Transactions[0].Outputs[0].AssetID
	height, err := DefaultLedger.Store.InitWithGenesisBlock(genesisBlock)
	if err != nil {
		return errors.New("[Blockchain], InitLevelDBStoreWithGenesisBlock failed.")
	}

	DefaultLedger.Blockchain.UpdateBestHeight(height)
	return nil
}

func GetGenesisBlock() (*Block, error) {
	// header
	header := &Header{
		Version:    BlockVersion,
		Previous:   EmptyHash,
		MerkleRoot: EmptyHash,
		Timestamp:  uint32(time.Unix(time.Date(2017, time.December, 22, 10, 0, 0, 0, time.UTC).Unix(), 0).Unix()),
		Bits:       0x1d03ffff,
		Nonce:      GenesisNonce,
		Height:     uint32(0),
	}

	// transaction
	systemToken := &Transaction{
		TxType:         RegisterAsset,
		PayloadVersion: 0,
		Payload: &PayloadRegisterAsset{
			Asset: Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: Uint168{},
		},
		Attributes: []*Attribute{},
		Inputs:     []*Input{},
		Outputs:    []*Output{},
		Programs:   []*Program{},
	}

	foundation, err := Uint168FromAddress(FoundationAddress)
	if err != nil {
		return nil, err
	}

	coinBase := NewCoinBaseTransaction(&PayloadCoinBase{}, 0)
	coinBase.Outputs = []*Output{
		{
			AssetID:     systemToken.Hash(),
			Value:       3300 * 10000 * 100000000,
			ProgramHash: *foundation,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := NewAttribute(Nonce, nonce)
	coinBase.Attributes = append(coinBase.Attributes, &txAttr)

	//block
	block := &Block{
		Header:       header,
		Transactions: []*Transaction{coinBase, systemToken},
	}
	hashes := make([]Uint256, 0, len(block.Transactions))
	for _, tx := range block.Transactions {
		hashes = append(hashes, tx.Hash())
	}
	block.Header.MerkleRoot, err = crypto.ComputeRoot(hashes)
	if err != nil {
		return nil, errors.New("[GenesisBlock] ,BuildMerkleRoot failed.")
	}

	return block, nil
}

func NewCoinBaseTransaction(coinBasePayload *PayloadCoinBase, currentHeight uint32) *Transaction {
	return &Transaction{
		TxType:         CoinBase,
		PayloadVersion: PayloadCoinBaseVersion,
		Payload:        coinBasePayload,
		Inputs: []*Input{
			{
				Previous: OutPoint{
					TxID:  EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*Attribute{},
		LockTime:   currentHeight,
		Programs:   []*Program{},
	}
}

func (bc *Blockchain) GetBestHeight() uint32 {
	bc.mutex.RLock()
	defer bc.mutex.RUnlock()
	return bc.BlockHeight
}

func (bc *Blockchain) UpdateBestHeight(val uint32) {
	//bc.mutex.Lock()
	//defer bc.mutex.Unlock()
	bc.BlockHeight = val
}

func (bc *Blockchain) AddBlock(block *Block) (bool, bool, error) {
	log.Debug()
	bc.mutex.Lock()
	defer bc.mutex.Unlock()

	noflags := uint32(0)
	inMainChain, isOrphan, err := bc.ProcessBlock(block, bc.TimeSource, noflags)
	if err != nil {
		return false, false, err
	}

	return inMainChain, isOrphan, nil
}

func (bc *Blockchain) GetHeader(hash Uint256) (*Header, error) {
	header, err := DefaultLedger.Store.GetHeader(hash)
	if err != nil {
		return nil, errors.New("[Blockchain], GetHeader failed.")
	}
	return header, nil
}

func (bc *Blockchain) ContainsTransaction(hash Uint256) bool {
	//TODO: implement error catch
	_, _, err := DefaultLedger.Store.GetTransaction(hash)
	if err != nil {
		return false
	}
	return true
}

func (bc *Blockchain) CurrentBlockHash() Uint256 {
	return DefaultLedger.Store.GetCurrentBlockHash()
}

type OrphanBlock struct {
	Block      *Block
	Expiration time.Time
}

func (bc *Blockchain) ProcessOrphans(hash *Uint256) error {
	processHashes := make([]*Uint256, 0, 10)
	processHashes = append(processHashes, hash)
	for len(processHashes) > 0 {
		processHash := processHashes[0]
		processHashes[0] = nil // Prevent GC leak.
		processHashes = processHashes[1:]

		for i := 0; i < len(bc.PrevOrphans[*processHash]); i++ {
			orphan := bc.PrevOrphans[*processHash][i]
			if orphan == nil {
				continue
			}

			orphanHash := orphan.Block.Hash()
			bc.RemoveOrphanBlock(orphan)
			i--

			//log.Trace("deal with orphan block %x", orphanHash.ToArrayReverse())
			_, err := bc.maybeAcceptBlock(orphan.Block)
			if err != nil {
				return err
			}

			processHashes = append(processHashes, &orphanHash)
		}
	}
	return nil
}

func (bc *Blockchain) RemoveOrphanBlock(orphan *OrphanBlock) {
	bc.OrphanLock.Lock()
	defer bc.OrphanLock.Unlock()

	orphanHash := orphan.Block.Hash()
	delete(bc.Orphans, orphanHash)

	prevHash := &orphan.Block.Header.Previous
	orphans := bc.PrevOrphans[*prevHash]
	for i := 0; i < len(orphans); i++ {
		hash := orphans[i].Block.Hash()
		if hash.IsEqual(orphanHash) {
			copy(orphans[i:], orphans[i+1:])
			orphans[len(orphans)-1] = nil
			orphans = orphans[:len(orphans)-1]
			i--
		}
	}
	bc.PrevOrphans[*prevHash] = orphans

	if len(bc.PrevOrphans[*prevHash]) == 0 {
		delete(bc.PrevOrphans, *prevHash)
	}
}

func (bc *Blockchain) AddOrphanBlock(block *Block) {
	for _, oBlock := range bc.Orphans {
		if time.Now().After(oBlock.Expiration) {
			bc.RemoveOrphanBlock(oBlock)
			if bc.OldestOrphan == oBlock {
				bc.OldestOrphan = nil
			}
			continue
		}
	}

	for _, oBlock := range bc.Orphans {
		if bc.OldestOrphan == nil || oBlock.Expiration.Before(bc.OldestOrphan.Expiration) {
			bc.OldestOrphan = oBlock
		}
	}

	if len(bc.Orphans)+1 > maxOrphanBlocks {
		bc.RemoveOrphanBlock(bc.OldestOrphan)
		bc.OldestOrphan = nil
	}

	bc.OrphanLock.Lock()
	defer bc.OrphanLock.Unlock()

	// Insert the block into the orphan map with an expiration time
	// 1 hour from now.
	expiration := time.Now().Add(time.Hour)
	oBlock := &OrphanBlock{
		Block:      block,
		Expiration: expiration,
	}
	bc.Orphans[block.Hash()] = oBlock

	// Add to previous hash lookup index for faster dependency lookups.
	prevHash := &block.Header.Previous
	bc.PrevOrphans[*prevHash] = append(bc.PrevOrphans[*prevHash], oBlock)

	return
}

func (bc *Blockchain) IsKnownOrphan(hash *Uint256) bool {
	bc.OrphanLock.RLock()
	defer bc.OrphanLock.RUnlock()

	if _, exists := bc.Orphans[*hash]; exists {
		return true
	}

	return false
}

func (bc *Blockchain) GetOrphanRoot(hash *Uint256) *Uint256 {
	bc.OrphanLock.RLock()
	defer bc.OrphanLock.RUnlock()

	orphanRoot := hash
	prevHash := hash
	for {
		orphan, exists := bc.Orphans[*prevHash]
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

func NewBlockNode(blockHeader *Header, blockSha *Uint256) *BlockNode {
	//prevHash := blockHeader.Previous

	var prevhash, hash Uint256
	copy(hash[:], blockSha[:])
	copy(prevhash[:], blockHeader.Previous[:])
	node := BlockNode{
		Hash:       &hash,
		ParentHash: &prevhash,
		Height:     blockHeader.Height,
		Version:    blockHeader.Version,
		Bits:       blockHeader.Bits,
		Timestamp:  blockHeader.Timestamp,
		WorkSum:    CalcWork(blockHeader.Bits),
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

func (bc *Blockchain) LoadBlockNode(blockHeader *Header, hash *Uint256) (*BlockNode, error) {

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
	//if parentNode, ok := bc.Index[*prevHash]; ok {
	if parentNode, ok := bc.LookupNodeInIndex(prevHash); ok {
		// Case 1 -- This node is a child of an existing block node.
		// Update the node's work sum with the sum of the parent node's
		// work sum and this node's work, append the node as a child of
		// the parent node and set this node's parent to the parent
		// node.
		node.WorkSum = node.WorkSum.Add(parentNode.WorkSum, node.WorkSum)
		parentNode.Children = append(parentNode.Children, node)
		node.Parent = parentNode

	} else if childNodes, ok := bc.DepNodes[*hash]; ok {
		// Case 2 -- This node is the parent of one or more nodes.
		// Connect this block node to all of its children and update
		// all of the children (and their children) with the new work
		// sums.
		for _, childNode := range childNodes {
			childNode.Parent = node
			node.Children = append(node.Children, childNode)
			childNode.WorkSum.Add(childNode.WorkSum, node.WorkSum)
			AddChildrenWork(childNode, node.WorkSum)
			bc.Root = node
		}

	} else {
		// Case 3 -- The node does't have a parent and is not the parent
		// of another node.  This is only acceptable for the first node
		// inserted into the chain.  Otherwise it means an arbitrary
		// orphan block is trying to be loaded which is not allowed.
		if bc.Root != nil {
			str := "LoadBlockNode: attempt to insert orphan block %v"
			return nil, fmt.Errorf(str, hash)
		}

		// Case 4 -- This is the root since it's the first and only node.
		bc.Root = node
	}

	// Add the new node to the indices for faster lookups.
	//bc.Index[*hash] = node
	bc.AddNodeToIndex(node)
	bc.DepNodes[*prevHash] = append(bc.DepNodes[*prevHash], node)

	return node, nil
}

func (bc *Blockchain) PruneBlockNodes() error {
	if bc.BestChain == nil {
		return nil
	}

	newRootNode := bc.BestChain
	for i := uint32(0); i < MinMemoryNodes-1 && newRootNode != nil; i++ {
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
		err := bc.RemoveBlockNode(node)
		if err != nil {
			return err
		}
	}

	// Set the new root node.
	bc.Root = newRootNode

	return nil
}

func (bc *Blockchain) RemoveBlockNode(node *BlockNode) error {
	if node.Parent != nil {
		return fmt.Errorf("RemoveBlockNode must be called with a "+
			" node at the front of the chain - node %v", node.Hash)
	}

	// Remove the node from the node index.
	//delete(bc.Index, *node.Hash)
	bc.RemoveNodeFromIndex(node)

	// Unlink all of the node's children.
	for _, child := range node.Children {
		child.Parent = nil
	}
	node.Children = nil

	// Remove the reference from the dependency index.
	prevHash := node.ParentHash
	if children, ok := bc.DepNodes[*prevHash]; ok {
		// Find the node amongst the children of the
		// dependencies for the parent hash and remove it.
		bc.DepNodes[*prevHash] = RemoveChildNode(children, node)

		// Remove the map entry altogether if there are no
		// longer any nodes which depend on the parent hash.
		if len(bc.DepNodes[*prevHash]) == 0 {
			delete(bc.DepNodes, *prevHash)
		}
	}

	return nil
}

// getPrevNodeFromBlock returns a block node for the block previous to the
// passed block (the passed block's parent).  When it is already in the memory
// block chain, it simply returns it.  Otherwise, it loads the previous block
// from the block database, creates a new block node from it, and returns it.
// The returned node will be nil if the genesis block is passed.
func (bc *Blockchain) GetPrevNodeFromBlock(block *Block) (*BlockNode, error) {
	// Genesis block.
	prevHash := block.Header.Previous
	if prevHash.IsEqual(EmptyHash) {
		return nil, nil
	}

	// Return the existing previous block node if it's already there.
	//if bn, ok := bc.Index[*prevHash]; ok {
	if bn, ok := bc.LookupNodeInIndex(&prevHash); ok {
		return bn, nil
	}

	header, err := bc.GetHeader(prevHash)
	if err != nil {
		return nil, err
	}
	prevBlockNode, err := bc.LoadBlockNode(header, &prevHash)
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
func (bc *Blockchain) GetPrevNodeFromNode(node *BlockNode) (*BlockNode, error) {
	// Return the existing previous block node if it's already there.
	if node.Parent != nil {
		return node.Parent, nil
	}

	// Genesis block.
	if node.Hash.IsEqual(bc.GenesisHash) {
		return nil, nil
	}

	header, err := bc.GetHeader(*node.ParentHash)
	if err != nil {
		return nil, err
	}
	prevBlockNode, err := bc.LoadBlockNode(header, node.ParentHash)
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
func (bc *Blockchain) GetReorganizeNodes(node *BlockNode) (*list.List, *list.List) {
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
	for n := bc.BestChain; n != nil && n.Parent != nil; n = n.Parent {
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
func (bc *Blockchain) ReorganizeChain(detachNodes, attachNodes *list.List) error {
	// Ensure all of the needed side chain blocks are in the cache.
	for e := attachNodes.Front(); e != nil; e = e.Next() {
		n := e.Value.(*BlockNode)
		if _, exists := bc.BlockCache[*n.Hash]; !exists {
			return fmt.Errorf("block %x is missing from the side "+
				"chain block cache", n.Hash.Bytes())
		}
	}

	// Perform several checks to verify each block that needs to be attached
	// to the main chain can be connected without violating any rules and
	// without actually connecting the block.
	//
	// NOTE: bitcoind does these checks directly when it connects a block.
	// The downside to that approach is that if any of these checks fail
	// after disconnecting some blocks or attaching others, all of the
	// operations have to be rolled back to get the chain back into the
	// state it was before the rule violation (or other failure).  There are
	// at least a couple of ways accomplish that rollback, but both involve
	// tweaking the chain.  This approach catches these issues before ever
	// modifying the chain.
	//TODO add checkConnectBlock
	//for e := attachNodes.Front(); e != nil; e = e.Next() {
	//	n := e.Value.(*BlockNode)
	//	block := s.BlockCache[*n.Hash]
	//	err := s.checkConnectBlock(n, block)
	//	if err != nil {
	//		return err
	//	}
	//}

	// Disconnect blocks from the main chain.
	for e := detachNodes.Front(); e != nil; e = e.Next() {
		n := e.Value.(*BlockNode)
		block, err := DefaultLedger.Store.GetBlock(*n.Hash)
		if err != nil {
			return err
		}
		err = bc.DisconnectBlock(n, block)
		if err != nil {
			return err
		}
	}

	// Connect the new best chain blocks.
	for e := attachNodes.Front(); e != nil; e = e.Next() {
		n := e.Value.(*BlockNode)
		block := bc.BlockCache[*n.Hash]
		err := bc.ConnectBlock(n, block)
		if err != nil {
			return err
		}
		delete(bc.BlockCache, *n.Hash)
	}

	// Log the point where the chain forked.
	//firstAttachNode := attachNodes.Front().Value.(*BlockNode)
	//forkNode, err := bc.GetPrevNodeFromNode(firstAttachNode)
	//if err == nil {
	//	log.Tracef("REORGANIZE: Chain forks at %v", forkNode.Hash)
	//}

	//// Log the old and new best chain heads.
	//firstDetachNode := detachNodes.Front().Value.(*BlockNode)
	//lastAttachNode := attachNodes.Back().Value.(*BlockNode)
	//log.Tracef("REORGANIZE: Old best chain head was %v", firstDetachNode.Hash)
	//log.Tracef("REORGANIZE: New best chain head is %v", lastAttachNode.Hash)

	return nil
}

//// disconnectBlock handles disconnecting the passed node/block from the end of
//// the main (best) chain.
func (bc *Blockchain) DisconnectBlock(node *BlockNode, block *Block) error {
	// Make sure the node being disconnected is the end of the best chain.
	if bc.BestChain == nil || !node.Hash.IsEqual(*bc.BestChain.Hash) {
		return fmt.Errorf("disconnectBlock must be called with the " +
			"block at the end of the main chain")
	}

	// Remove the block from the database which houses the main chain.
	_, err := bc.GetPrevNodeFromNode(node)
	if err != nil {
		return err
	}

	err = DefaultLedger.Store.RollbackBlock(*node.Hash)
	if err != nil {
		return err
	}

	// Put block in the side chain cache.
	node.InMainChain = false
	bc.BlockCache[*node.Hash] = block

	//// This node's parent is now the end of the best chain.
	bc.BestChain = node.Parent
	bc.MedianTimePast = CalcPastMedianTime(bc.BestChain)

	// Notify the caller that the block was disconnected from the main
	// chain.  The caller would typically want to react with actions such as
	// updating wallets.
	//TODO
	//bc.sendNotification(NTBlockDisconnected, block)

	return nil
}

// connectBlock handles connecting the passed node/block to the end of the main
// (best) chain.
func (bc *Blockchain) ConnectBlock(node *BlockNode, block *Block) error {

	for _, txVerify := range block.Transactions {
		if errCode := CheckTransactionContext(txVerify); errCode != Success {
			fmt.Println("CheckTransactionContext failed when verifiy block", errCode)
			return errors.New(fmt.Sprintf("CheckTransactionContext failed when verifiy block"))
		}
	}

	// Make sure it's extending the end of the best chain.
	prevHash := &block.Header.Previous
	if bc.BestChain != nil && !prevHash.IsEqual(*bc.BestChain.Hash) {
		return fmt.Errorf("connectBlock must be called with a block " +
			"that extends the main chain")
	}

	// Insert the block into the database which houses the main chain.
	err := DefaultLedger.Store.SaveBlock(block)
	if err != nil {
		return err
	}

	// Add the new node to the memory main chain indices for faster
	// lookups.
	node.InMainChain = true
	//bc.Index[*node.Hash] = node
	bc.AddNodeToIndex(node)
	bc.DepNodes[*prevHash] = append(bc.DepNodes[*prevHash], node)

	// This node is now the end of the best chain.
	bc.BestChain = node
	bc.MedianTimePast = CalcPastMedianTime(bc.BestChain)

	// Notify the caller that the block was connected to the main chain.
	// The caller would typically want to react with actions such as
	// updating wallets.
	//TODO
	//bc.sendNotification(NTBlockConnected, block)

	return nil
}

func (bc *Blockchain) BlockExists(hash *Uint256) (bool, error) {
	// Check memory chain first (could be main chain or side chain blocks).
	//if _, ok := bc.Index[*hash]; ok {
	if _, ok := bc.LookupNodeInIndex(hash); ok {
		return true, nil
	}

	// Check in database (rest of main chain not in memory).
	return DefaultLedger.BlockInLedger(*hash), nil
	return false, nil
}

func (bc *Blockchain) maybeAcceptBlock(block *Block) (bool, error) {

	// Get a block node for the block previous to this one.  Will be nil
	// if this is the genesis block.
	prevNode, err := bc.GetPrevNodeFromBlock(block)
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

	// The block must pass all of the validation rules which depend on the
	// position of the block within the block chain.
	err = PowCheckBlockContext(block, prevNode, DefaultLedger)
	if err != nil {
		log.Error("PowCheckBlockContext error!", err)
		return false, err
	}

	// Prune block nodes which are no longer needed before creating
	// a new node.
	err = bc.PruneBlockNodes()
	if err != nil {
		return false, err
	}

	// Create a new block node for the block and add it to the in-memory
	// block chain (could be either a side chain or the main chain).
	blockHeader := block.Header
	blockhash := block.Hash()
	newNode := NewBlockNode(blockHeader, &blockhash)
	if prevNode != nil {
		newNode.Parent = prevNode
		newNode.Height = blockHeight
		newNode.WorkSum.Add(prevNode.WorkSum, newNode.WorkSum)
	}

	// Connect the passed block to the chain while respecting proper chain
	// selection according to the chain with the most proof of work.  This
	// also handles validation of the transaction scripts.
	inMainChain, err := bc.ConnectBestChain(newNode, block)
	if err != nil {
		return false, err
	}

	//// Notify the caller that the new block was accepted into the block
	//// chain.  The caller would typically want to react by relaying the
	//// inventory to other peers.
	////b.sendNotification(NTBlockAccepted, block)

	return inMainChain, nil
}

func (bc *Blockchain) ConnectBestChain(node *BlockNode, block *Block) (bool, error) {
	// We haven't selected a best chain yet or we are extending the main
	// (best) chain with a new block.  This is the most common case.

	if bc.BestChain == nil || (node.Parent.Hash.IsEqual(*bc.BestChain.Hash)) {
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
		err := bc.ConnectBlock(node, block)
		if err != nil {
			return false, err
		}

		// Connect the parent node to this node.
		if node.Parent != nil {
			node.Parent.Children = append(node.Parent.Children, node)
		}

		return true, nil
	}

	// We're extending (or creating) a side chain which may or may not
	// become the main chain, but in either case we need the block stored
	// for future processing, so add the block to the side chain holding
	// cache.
	log.Debugf("Adding block %x to side chain cache", node.Hash.Bytes())
	bc.BlockCache[*node.Hash] = block
	//bc.Index[*node.Hash] = node
	bc.AddNodeToIndex(node)

	// Connect the parent node to this node.
	node.InMainChain = false
	node.Parent.Children = append(node.Parent.Children, node)

	// We're extending (or creating) a side chain, but the cumulative
	// work for this new side chain is not enough to make it the new chain.
	if node.WorkSum.Cmp(bc.BestChain.WorkSum) <= 0 {

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

		return false, nil
	}

	// We're extending (or creating) a side chain and the cumulative work
	// for this new side chain is more than the old best chain, so this side
	// chain needs to become the main chain.  In order to accomplish that,
	// find the common ancestor of both sides of the fork, disconnect the
	// blocks that form the (now) old fork from the main chain, and attach
	// the blocks that form the new chain to the main chain starting at the
	// common ancenstor (the point where the chain forked).
	detachNodes, attachNodes := bc.GetReorganizeNodes(node)
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
	err := bc.ReorganizeChain(detachNodes, attachNodes)
	if err != nil {
		return false, err
	}

	return true, nil
}

//(bool, bool, error)
//1. inMainChain
//2. isOphan
//3. error
func (bc *Blockchain) ProcessBlock(block *Block, timeSource MedianTimeSource, flags uint32) (bool, bool, error) {
	blockHash := block.Hash()
	log.Tracef("[ProcessBLock] height = %d, hash = %x", block.Header.Height, blockHash.Bytes())

	// The block must not already exist in the main chain or side chains.
	exists, err := bc.BlockExists(&blockHash)
	if err != nil {
		log.Trace("[ProcessBLock] block exists err")
		return false, false, err
	}
	if exists {
		str := fmt.Sprintf("already have block %x\n", blockHash.Bytes())
		return false, false, fmt.Errorf(str)
	}

	// The block must not already exist as an orphan.
	if _, exists := bc.Orphans[blockHash]; exists {
		str := fmt.Sprintf("already have block (orphan) %v", blockHash)
		return false, false, fmt.Errorf(str)
	}

	log.Tracef("[ProcessBLock] orphan already exist= %v", exists)

	// Perform preliminary sanity checks on the block and its transactions.
	//err = PowCheckBlockSanity(block, PowLimit, bc.TimeSource)
	err = PowCheckBlockSanity(block, config.Parameters.ChainParam.PowLimit, bc.TimeSource)

	if err != nil {
		log.Error("PowCheckBlockSanity error!")
		return false, false, err
	}

	blockHeader := block.Header

	// Handle orphan blocks.
	prevHash := blockHeader.Previous
	if !prevHash.IsEqual(EmptyHash) {
		prevHashExists, err := bc.BlockExists(&prevHash)
		if err != nil {
			return false, false, err
		}
		//log.Tracef("[ProcessBLock] prev block already exist= %v\n", prevHashExists)
		if !prevHashExists {
			log.Tracef("Adding orphan block %x with parent %x", blockHash.Bytes(), prevHash.Bytes())
			bc.AddOrphanBlock(block)

			return false, true, nil
		}
	}

	//The block has passed all context independent checks and appears sane
	//enough to potentially accept it into the block chain.
	inMainChain, err := bc.maybeAcceptBlock(block)
	if err != nil {
		return false, true, err
	}

	// Accept any orphan blocks that depend on this block (they are
	// no longer orphans) and repeat for those accepted blocks until
	// there are no more.
	err = bc.ProcessOrphans(&blockHash)
	if err != nil {
		//TODO inMainChain or not
		return false, false, err
	}

	//log.Debugf("Accepted block %v", blockHash)

	return inMainChain, false, nil
}

func (bc *Blockchain) DumpState() {
	log.Trace("bc.BestChain=", bc.BestChain.Hash)
	log.Trace("bc.Root=", bc.Root.Hash)
	//log.Trace("bc.Index=", bc.Index)
	//log.Trace("bc.DepNodes=", bc.DepNodes)
	//for _, nd := range bc.Orphans {
	//	log.Trace(nd)
	//}
	//	for _, nd := range bc.Index {
	//		DumpBlockNode(nd)
	//	}
}

func DumpBlockNode(node *BlockNode) {
	log.Tracef("---------------------node:%p\n", node)
	log.Trace("Hash:", node.Hash)
	log.Trace("ParentHash:", node.ParentHash)
	log.Trace("Height:", node.Height)
	log.Trace("Version:", node.Version)
	log.Trace("Bits:", node.Bits)
	log.Trace("Timestamp:", node.Timestamp)
	log.Trace("WorkSum:", node.WorkSum)
	log.Trace("InMainChain:", node.InMainChain)
	if node.Parent != nil {
		log.Trace("Parent:", node.Parent.Hash)
	} else {
		log.Trace("Parent:", node.Parent)
	}
	log.Trace("Children:", node.Children)
	log.Trace("---------------------")
}

func (b *Blockchain) LatestBlockLocator() ([]*Uint256, error) {
	b.mutex.RLock()
	defer b.mutex.RUnlock()
	if b.BestChain == nil {
		// Get the latest block hash for the main chain from the
		// database.

		// Get Current Block
		blockHash := DefaultLedger.Store.GetCurrentBlockHash()
		return b.blockLocatorFromHash(&blockHash), nil
	}

	// The best chain is set, so use its hash.
	return b.blockLocatorFromHash(b.BestChain.Hash), nil
}
func (b *Blockchain) AddNodeToIndex(node *BlockNode) {
	b.IndexLock.Lock()
	defer b.IndexLock.Unlock()

	b.Index[*node.Hash] = node
}
func (b *Blockchain) RemoveNodeFromIndex(node *BlockNode) {
	b.IndexLock.Lock()
	defer b.IndexLock.Unlock()
	delete(b.Index, *node.Hash)
}
func (b *Blockchain) LookupNodeInIndex(hash *Uint256) (*BlockNode, bool) {
	b.IndexLock.Lock()
	defer b.IndexLock.Unlock()
	node, exist := b.Index[*hash]

	return node, exist
}

func (b *Blockchain) BlockLocatorFromHash(inhash *Uint256) []*Uint256 {
	b.mutex.RLock()
	defer b.mutex.RUnlock()
	return b.blockLocatorFromHash(inhash)
}
func (b *Blockchain) blockLocatorFromHash(inhash *Uint256) []*Uint256 {
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

		block, err := DefaultLedger.Store.GetBlock(hash)
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
	for len(locator) < MaxBlockLocatorsPerMsg-1 {
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

		h, err := DefaultLedger.Store.GetBlockHash(uint32(blockHeight))
		if err != nil {
			log.Tracef("Lookup of known valid height failed %v", blockHeight)
			continue
		}

		locator = append(locator, &h)
	}

	// Append the appropriate genesis block.
	locator = append(locator, &b.GenesisHash)

	return locator
}

func (b *Blockchain) LatestLocatorHash(locator []*Uint256) *Uint256 {
	var startHash Uint256
	for _, hash := range locator {
		_, err := DefaultLedger.Store.GetBlock(*hash)
		if err == nil {
			startHash = *hash
			break
		}
	}
	return &startHash
}

func (b *Blockchain) MedianAdjustedTime() time.Time {
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
