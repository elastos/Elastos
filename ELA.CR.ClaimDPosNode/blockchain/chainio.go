// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

// Copyright (c) 2013-2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"
)

const (
	// HashSize of array used to store hashes.  See Hash.
	HashSize = 32

	// blockHdrNoAuxSize is the size of block header without aux.
	blockHdrNoAuxSize = 84

	// latestUtxoSetBucketVersion is the current version of the utxo set
	// bucket that is used to track all unspent outputs.
	latestUtxoSetBucketVersion = 2

	// latestSpendJournalBucketVersion is the current version of the spend
	// journal bucket that is used to track all spent transactions for use
	// in reorgs.
	latestSpendJournalBucketVersion = 1

	// approxNodesPerWeek is an approximation of the number of new blocks there
	// are in a week on average.
	approxNodesPerWeek = 30 * 24 * 7
)

var (
	// blockIndexBucketName is the name of the db bucket used to house to the
	// block headers and contextual information.
	blockIndexBucketName = []byte("blockheaderidx")

	// hashIndexBucketName is the name of the db bucket used to house to the
	// block hash -> block height index.
	hashIndexBucketName = []byte("hashidx")

	// heightIndexBucketName is the name of the db bucket used to house to
	// the block height -> block hash index.
	heightIndexBucketName = []byte("heightidx")

	// chainStateKeyName is the name of the db key used to store the best
	// chain state.
	chainStateKeyName = []byte("chainstate")

	// spendJournalVersionKeyName is the name of the db key used to store
	// the version of the spend journal currently in the database.
	spendJournalVersionKeyName = []byte("spendjournalversion")

	// spendJournalBucketName is the name of the db bucket used to house
	// transactions outputs that are spent in each block.
	spendJournalBucketName = []byte("spendjournal")

	// utxoSetVersionKeyName is the name of the db key used to store the
	// version of the utxo set currently in the database.
	utxoSetVersionKeyName = []byte("utxosetversion")

	// utxoSetBucketName is the name of the db bucket used to house the
	// unspent transaction output set.
	utxoSetBucketName = []byte("utxosetv2")

	// byteOrder is the preferred byte order used for serializing numeric
	// fields for storage in the database.
	byteOrder = binary.LittleEndian
)

// BestState houses information about the current best block and other info
// related to the state of the main chain as it exists from the point of view of
// the current best block.
//
// The BestSnapshot method can be used to obtain access to this information
// in a concurrent safe manner and the data will not be changed out from under
// the caller when chain state changes occur as the function name implies.
// However, the returned snapshot must be treated as immutable since it is
// shared by all callers.
type BestState struct {
	Hash        common.Uint256 // The hash of the block.
	Height      uint32         // The height of the block.
	Bits        uint32         // The difficulty bits of the block.
	BlockSize   uint64         // The size of the block.
	BlockWeight uint64         // The weight of the block.
	NumTxns     uint64         // The number of txns in the block.
	MedianTime  time.Time      // Median time as per CalcPastMedianTime.
}

// newBestState returns a new best stats instance for the given parameters.
func newBestState(node *BlockNode, blockSize, blockWeight, numTxns uint64,
	medianTime time.Time) *BestState {

	return &BestState{
		Hash:        *node.Hash,
		Height:      node.Height,
		Bits:        node.Bits,
		BlockSize:   blockSize,
		BlockWeight: blockWeight,
		NumTxns:     numTxns,
		MedianTime:  medianTime,
	}
}

// dbPutBestState uses an existing database transaction to update the best chain
// state with the given parameters.
func dbPutBestState(dbTx database.Tx, snapshot *BestState, workSum *big.Int) error {
	// Serialize the current best chain state.
	serializedData := serializeBestChainState(bestChainState{
		hash:    snapshot.Hash,
		height:  uint32(snapshot.Height),
		workSum: workSum,
	})

	// Store the current best chain state into the database.
	return dbTx.Metadata().Put(chainStateKeyName, serializedData)
}

// -----------------------------------------------------------------------------
// The best chain state consists of the best block hash and height, the total
// number of transactions up to and including those in the best block, and the
// accumulated work sum up to and including the best block.
//
// The serialized format is:
//
//   <block hash><block height><total txns><work sum length><work sum>
//
//   Field             Type             Size
//   block hash        chainhash.Hash   chainhash.HashSize
//   block height      uint32           4 bytes
//   total txns        uint64           8 bytes
//   work sum length   uint32           4 bytes
//   work sum          big.Int          work sum length
// -----------------------------------------------------------------------------

// bestChainState represents the data to be stored the database for the current
// best chain state.
type bestChainState struct {
	hash      common.Uint256
	height    uint32
	totalTxns uint64
	workSum   *big.Int
}

// serializeBestChainState returns the serialization of the passed block best
// chain state.  This is data to be stored in the chain state bucket.
func serializeBestChainState(state bestChainState) []byte {
	// Calculate the full size needed to serialize the chain state.
	workSumBytes := state.workSum.Bytes()
	workSumBytesLen := uint32(len(workSumBytes))
	serializedLen := HashSize + 4 + 8 + 4 + workSumBytesLen

	// Serialize the chain state.
	serializedData := make([]byte, serializedLen)
	copy(serializedData[0:HashSize], state.hash[:])
	offset := uint32(HashSize)
	byteOrder.PutUint32(serializedData[offset:], state.height)
	offset += 4
	byteOrder.PutUint64(serializedData[offset:], state.totalTxns)
	offset += 8
	byteOrder.PutUint32(serializedData[offset:], workSumBytesLen)
	offset += 4
	copy(serializedData[offset:], workSumBytes)
	return serializedData[:]
}

// deserializeBestChainState deserializes the passed serialized best chain
// state.  This is data stored in the chain state bucket and is updated after
// every block is connected or disconnected form the main chain.
// block.
func deserializeBestChainState(serializedData []byte) (bestChainState, error) {
	// Ensure the serialized data has enough bytes to properly deserialize
	// the hash, height, total transactions, and work sum length.
	if len(serializedData) < HashSize+16 {
		return bestChainState{}, database.Error{
			ErrorCode:   database.ErrCorruption,
			Description: "corrupt best chain state",
		}
	}

	state := bestChainState{}
	copy(state.hash[:], serializedData[0:HashSize])
	offset := uint32(HashSize)
	state.height = byteOrder.Uint32(serializedData[offset : offset+4])
	offset += 4
	state.totalTxns = byteOrder.Uint64(serializedData[offset : offset+8])
	offset += 8
	workSumBytesLen := byteOrder.Uint32(serializedData[offset : offset+4])
	offset += 4

	// Ensure the serialized data has enough bytes to deserialize the work
	// sum.
	if uint32(len(serializedData[offset:])) < workSumBytesLen {
		return bestChainState{}, database.Error{
			ErrorCode:   database.ErrCorruption,
			Description: "corrupt best chain state",
		}
	}
	workSumBytes := serializedData[offset : offset+workSumBytesLen]
	state.workSum = new(big.Int).SetBytes(workSumBytes)

	return state, nil
}

// dbStoreBlock stores the provided block in the database if it is not already
// there. The full block data is written to fflDB.
func dbStoreBlock(dbTx database.Tx, block *types.DposBlock) error {
	blockHash := block.Hash()
	hasBlock, err := dbTx.HasBlock(blockHash)
	if err != nil {
		return err
	}
	if hasBlock {
		return nil
	}
	return dbTx.StoreBlock(block)
}

// -----------------------------------------------------------------------------
// The block index consists of two buckets with an entry for every block in the
// main chain.  One bucket is for the hash to height mapping and the other is
// for the height to hash mapping.
//
// The serialized format for values in the hash to height bucket is:
//   <height>
//
//   Field      Type     Size
//   height     uint32   4 bytes
//
// The serialized format for values in the height to hash bucket is:
//   <hash>
//
//   Field      Type             Size
//   hash       Hash   			 HashSize
// -----------------------------------------------------------------------------

// dbPutBlockIndex uses an existing database transaction to update or add the
// block index entries for the hash to height and height to hash mappings for
// the provided values.
func dbPutBlockIndex(dbTx database.Tx, hash *common.Uint256, height uint32) error {
	// Serialize the height for use in the index entries.
	var serializedHeight [4]byte
	byteOrder.PutUint32(serializedHeight[:], height)

	// Add the block hash to height mapping to the index.
	meta := dbTx.Metadata()
	hashIndex := meta.Bucket(hashIndexBucketName)
	if err := hashIndex.Put(hash[:], serializedHeight[:]); err != nil {
		return err
	}

	// Add the block height to hash mapping to the index.
	heightIndex := meta.Bucket(heightIndexBucketName)
	return heightIndex.Put(serializedHeight[:], hash[:])
}

// createChainState initializes both the database and the chain state to the
// genesis block.  This includes creating the necessary buckets and inserting
// the genesis block, so it must only be called on an uninitialized database.
func (b *BlockChain) createChainState() error {

	// Create a new node from the genesis block and set it as the best node.
	genesisBlock := b.chainParams.GenesisBlock
	header := &genesisBlock.Header
	hash := genesisBlock.Hash()

	// Create the new block node for the block and set the work.
	node := NewBlockNode(header, &hash)
	node.InMainChain = true
	node.Status = statusDataStored | statusValid

	b.BestChain = node

	// Add the new node to the index which is used for faster lookups.
	b.Nodes = append(b.Nodes, node)

	// Initialize the state related to the best block.  Since it is the
	// genesis block, use its timestamp for the median time.
	numTxns := uint64(len(genesisBlock.Transactions))
	blockSize := uint64(genesisBlock.GetSize())
	blockWeight := uint64(GetBlockWeight(genesisBlock))
	state := newBestState(node, blockSize, blockWeight, numTxns,
		time.Unix(int64(genesisBlock.Timestamp), 0))
	// Create the initial the database chain state including creating the
	// necessary index buckets and inserting the genesis block.
	err := b.db.GetFFLDB().Update(func(dbTx database.Tx) error {
		meta := dbTx.Metadata()

		// Create the bucket that houses the block index data.
		_, err := meta.CreateBucket(blockIndexBucketName)
		if err != nil {
			return err
		}

		// Create the bucket that houses the chain block hash to height
		// index.
		_, err = meta.CreateBucket(hashIndexBucketName)
		if err != nil {
			return err
		}

		// Create the bucket that houses the chain block height to hash
		// index.
		_, err = meta.CreateBucket(heightIndexBucketName)
		if err != nil {
			return err
		}

		// Create the bucket that houses the spend journal data and
		// store its version.
		_, err = meta.CreateBucket(spendJournalBucketName)
		if err != nil {
			return err
		}

		// Create the bucket that houses the utxo set and store its
		// version.  Note that the genesis block coinbase transaction is
		// intentionally not inserted here since it is not spendable by
		// consensus rules.
		_, err = meta.CreateBucket(utxoSetBucketName)
		if err != nil {
			return err
		}

		// Save the genesis block to the block index database.
		err = DBStoreBlockNode(dbTx, header, node.Status)
		if err != nil {
			return err
		}

		// Add the genesis block hash to height and height to hash
		// mappings to the index.
		err = dbPutBlockIndex(dbTx, node.Hash, node.Height)
		if err != nil {
			return err
		}

		// Store the current best chain state into the database.
		err = dbPutBestState(dbTx, state, node.WorkSum)
		if err != nil {
			return err
		}

		// Store the genesis block into the database.
		return dbStoreBlock(dbTx, &types.DposBlock{Block: genesisBlock})
	})
	return err
}

// initChainState attempts to load and initialize the chain state from the
// database.  When the db does not yet contain any chain state, both it and the
// chain state are initialized to the genesis block.
func (b *BlockChain) initChainState() error {
	// Determine the state of the chain database. We may need to initialize
	// everything from scratch or upgrade certain buckets.
	var initialized, hasBlockIndex bool
	err := b.db.GetFFLDB().View(func(dbTx database.Tx) error {
		initialized = dbTx.Metadata().Get(chainStateKeyName) != nil
		hasBlockIndex = dbTx.Metadata().Bucket(blockIndexBucketName) != nil
		return nil
	})
	if err != nil {
		return err
	}

	if !initialized {
		// At this point the database has not already been initialized, so
		// initialize both it and the chain state to the genesis block.
		return b.createChainState()
	}

	if !hasBlockIndex {
		//err = migrateBlockIndex(b.db.GetFFLDB())
		//if err != nil {
		//	return initialized, nil
		//}
		//return initialized, errors.New("initChainState failed")
		return nil
	}

	// Attempt to load the chain state from the database.
	err = b.db.GetFFLDB().View(func(dbTx database.Tx) error {
		// Load all of the headers from the data for the known best
		// chain and construct the block index accordingly.  Since the
		// number of nodes are already known, perform a single alloc
		// for them versus a whole bunch of little ones to reduce
		// pressure on the GC.
		log.Infof("Loading block index...")

		blockIndexBucket := dbTx.Metadata().Bucket(blockIndexBucketName)

		// Determine how many blocks will be loaded into the index so we can
		// allocate the right amount.
		var blockCount int32
		cursor := blockIndexBucket.Cursor()
		for ok := cursor.First(); ok; ok = cursor.Next() {
			blockCount++
		}
		log.Info("block count:", blockCount)

		var i int32
		var lastNode *BlockNode
		cursor = blockIndexBucket.Cursor()
		for ok := cursor.First(); ok; ok = cursor.Next() {
			header, status, err := DeserializeBlockRow(cursor.Value())
			if err != nil {
				return err
			}
			curHash := header.Hash()
			if lastNode == nil && !curHash.IsEqual(b.chainParams.GenesisBlock.Hash()) {
				return fmt.Errorf("initChainState: Expected "+
					"first entry in block index to be genesis block, "+
					"found %s", curHash)
			}

			// Initialize the block node for the block, connect it,
			// and add it to the block index.
			node, err := b.LoadBlockNode(header, &curHash)
			if err != nil {
				return fmt.Errorf("initChainState: Could "+
					"not load block node for block %s", header.Hash())
			}
			node.Status = status

			lastNode = node
			i++
		}

		// As a final consistency check, we'll run through all the nodes which
		// are ancestors of the current chain tip, and find the real tip.
		for iterNode := lastNode; iterNode != nil; iterNode = iterNode.Parent {
			if iterNode.Status.KnownValid() {
				b.setTip(iterNode)
				break
			}
		}
		b.BestChain = b.Nodes[len(b.Nodes)-1]
		if b.BestChain.Height > b.db.GetHeight() {
			b.db.SetHeight(b.BestChain.Height)
		}

		return nil
	})
	if err != nil {
		return err
	}

	//// As we might have updated the index after it was loaded, we'll
	//// attempt to flush the index to the DB. This will only result in a
	//// write if the elements are dirty, so it'll usually be a noop.
	//return b.index.flushToDB()
	return nil
}

// DeserializeBlockRow parses a value in the block index bucket into a block
// header and block Status bitfield.
func DeserializeBlockRow(blockRow []byte) (*types.Header, blockStatus, error) {
	buffer := bytes.NewReader(blockRow)

	var header types.Header
	err := header.DeserializeNoAux(buffer)
	if err != nil {
		return nil, statusNone, err
	}

	statusByte, err := buffer.ReadByte()
	if err != nil {
		return nil, statusNone, err
	}

	return &header, blockStatus(statusByte), nil
}

// DBStoreBlockNode stores the block header to the block index bucket.
// This overwrites the current entry if there exists one.
func DBStoreBlockNode(dbTx database.Tx, header *types.Header,
	status blockStatus) error {
	// Serialize block data to be stored.
	w := bytes.NewBuffer(make([]byte, 0, blockHdrNoAuxSize))
	err := header.SerializeNoAux(w)
	if err != nil {
		return err
	}
	err = w.WriteByte(byte(status))
	if err != nil {
		return err
	}
	value := w.Bytes()

	// Write block header data to block index bucket.
	blockHash := header.Hash()
	blockIndexBucket := dbTx.Metadata().Bucket(blockIndexBucketName)
	key := blockIndexKey(&blockHash, header.Height)
	return blockIndexBucket.Put(key, value)
}

// DBRemoveBlockNode stores the block header to the block index bucket.
// This overwrites the current entry if there exists one.
func DBRemoveBlockNode(dbTx database.Tx, header *types.Header) error {
	// Write block header data to block index bucket.
	blockHash := header.Hash()
	blockIndexBucket := dbTx.Metadata().Bucket(blockIndexBucketName)
	key := blockIndexKey(&blockHash, header.Height)
	return blockIndexBucket.Delete(key)
}

// blockIndexKey generates the binary key for an entry in the block index
// bucket. The key is composed of the block height encoded as a big-endian
// 32-bit unsigned int followed by the 32 byte block hash.
func blockIndexKey(blockHash *common.Uint256, blockHeight uint32) []byte {
	indexKey := make([]byte, HashSize+4)
	binary.BigEndian.PutUint32(indexKey[0:4], blockHeight)
	copy(indexKey[4:HashSize+4], blockHash[:])
	return indexKey
}

// dbFetchHeightByHash uses an existing database transaction to retrieve the
// height for the provided hash from the index.
func dbFetchHeightByHash(dbTx database.Tx, hash *common.Uint256) (uint32, error) {
	meta := dbTx.Metadata()
	hashIndex := meta.Bucket(hashIndexBucketName)
	serializedHeight := hashIndex.Get(hash[:])
	if serializedHeight == nil {
		return 0, fmt.Errorf("block %s is not in the main chain", hash)
	}

	return uint32(byteOrder.Uint32(serializedHeight)), nil
}

// dbFetchBlockByNode uses an existing database transaction to retrieve the
// raw block for the provided node, deserialize it, and return a btcutil.Block
// with the height set.
func dbFetchBlockByNode(dbTx database.Tx, node *BlockNode) (*types.Block, error) {
	// Load the raw block bytes from the database.
	blockBytes, err := dbTx.FetchBlock(node.Hash)
	if err != nil {
		return nil, err
	}

	// Create the encapsulated block and set the height appropriately.
	var block types.Block
	err = block.Deserialize(bytes.NewReader(blockBytes))
	if err != nil {
		return nil, err
	}

	return &block, nil
}

// DBRemoveBlockIndex uses an existing database transaction remove block index
// entries from the hash to height and height to hash mappings for the provided
// values.
func DBRemoveBlockIndex(dbTx database.Tx, hash *common.Uint256, height uint32) error {
	// Remove the block hash to height mapping.
	meta := dbTx.Metadata()
	hashIndex := meta.Bucket(hashIndexBucketName)
	if err := hashIndex.Delete(hash[:]); err != nil {
		return err
	}

	// Remove the block height to hash mapping.
	var serializedHeight [4]byte
	byteOrder.PutUint32(serializedHeight[:], height)
	heightIndex := meta.Bucket(heightIndexBucketName)
	return heightIndex.Delete(serializedHeight[:])
}
