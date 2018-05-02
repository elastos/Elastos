package core

import (
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type RequestQueueHandler interface {
	OnSendRequest(peer *net.Peer, reqType uint8, hash Uint256)
	OnRequestError(error)
	OnRequestFinished(*FinishedReqPool)
}

type RequestQueue struct {
	size             int
	peer             *net.Peer
	hashesQueue      chan Uint256
	blocksQueue      chan Uint256
	blockTxsQueue    chan Uint256
	blockReqsLock    *sync.Mutex
	blockRequests    map[Uint256]*Request
	blockTxsReqsLock *sync.Mutex
	blockTxsRequests map[Uint256]*BlockTxsRequest
	blockTxs         map[Uint256]Uint256
	finished         *FinishedReqPool
	handler          RequestQueueHandler
}

func NewRequestQueue(size int, handler RequestQueueHandler) *RequestQueue {
	queue := new(RequestQueue)
	queue.size = size
	queue.hashesQueue = make(chan Uint256, size)
	queue.blocksQueue = make(chan Uint256, size)
	queue.blockTxsQueue = make(chan Uint256, size)
	queue.blockReqsLock = new(sync.Mutex)
	queue.blockRequests = make(map[Uint256]*Request)
	queue.blockTxsReqsLock = new(sync.Mutex)
	queue.blockTxsRequests = make(map[Uint256]*BlockTxsRequest)
	queue.blockTxs = make(map[Uint256]Uint256)
	queue.finished = &FinishedReqPool{
		blocks:   make(map[Uint256]*bloom.MerkleBlock),
		requests: make(map[Uint256]*BlockTxsRequest),
	}
	queue.handler = handler

	go queue.start()
	return queue
}

func (queue *RequestQueue) start() {
	for hash := range queue.hashesQueue {
		queue.StartBlockRequest(queue.peer, hash)
	}
}

// This method will block when request queue is filled
func (queue *RequestQueue) PushHashes(peer *net.Peer, hashes []*Uint256) {
	queue.peer = peer
	for _, hash := range hashes {
		queue.hashesQueue <- *hash
	}
}

func (queue *RequestQueue) StartBlockRequest(peer *net.Peer, hash Uint256) {
	// Check if already in request queue or finished
	if queue.InBlockRequestQueue(hash) || queue.InFinishedPool(hash) {
		return
	}
	// Block the method when queue is filled
	queue.blocksQueue <- hash

	queue.blockReqsLock.Lock()
	// Create a new block request
	blockRequest := &Request{
		peer:    peer,
		hash:    hash,
		reqType: p2p.BlockData,
		handler: queue,
	}
	// Add to request queue
	queue.blockRequests[hash] = blockRequest
	// Start block request
	blockRequest.Start()

	queue.blockReqsLock.Unlock()
}

func (queue *RequestQueue) StartBlockTxsRequest(peer *net.Peer, block *bloom.MerkleBlock, txIds []*Uint256) {
	blockHash := block.Header.Hash()
	// No block transactions to request, notify request finished.
	if len(txIds) == 0 {
		// Notify request finished
		queue.OnRequestFinished(&BlockTxsRequest{
			BlockHash: blockHash,
			Block:     *block,
		})
		return
	}
	// Check if request already in queue
	if queue.InBlockTxsRequestQueue(blockHash) {
		return
	}
	// Block the method when queue is filled
	queue.blockTxsQueue <- blockHash

	queue.blockTxsReqsLock.Lock()
	txRequestQueue := make(map[Uint256]*Request)
	for _, txId := range txIds {
		// Mark txId related block
		queue.blockTxs[*txId] = blockHash
		// Start a tx request
		txRequest := &Request{
			peer:    peer,
			hash:    *txId,
			reqType: p2p.TxData,
			handler: queue,
		}
		txRequestQueue[*txId] = txRequest
		txRequest.Start()
	}

	blockTxsRequest := &BlockTxsRequest{
		BlockHash:      blockHash,
		Block:          *block,
		txRequestQueue: txRequestQueue,
	}

	queue.blockTxsRequests[blockHash] = blockTxsRequest
	queue.blockTxsReqsLock.Unlock()
}

func (queue *RequestQueue) InBlockRequestQueue(blockHash Uint256) bool {
	queue.blockReqsLock.Lock()
	defer queue.blockReqsLock.Unlock()

	_, ok := queue.blockRequests[blockHash]
	return ok
}

func (queue *RequestQueue) InBlockTxsRequestQueue(blockHash Uint256) bool {
	queue.blockTxsReqsLock.Lock()
	defer queue.blockTxsReqsLock.Unlock()

	_, ok := queue.blockTxsRequests[blockHash]
	return ok
}

func (queue *RequestQueue) InFinishedPool(blockHash Uint256) bool {
	_, ok := queue.finished.Contain(blockHash)
	return ok
}

func (queue *RequestQueue) IsRunning() bool {
	return len(queue.hashesQueue) > 0 || len(queue.blocksQueue) > 0 || len(queue.blockTxsQueue) > 0
}

func (queue *RequestQueue) OnSendRequest(peer *net.Peer, reqType uint8, hash Uint256) {
	queue.handler.OnSendRequest(peer, reqType, hash)
}

func (queue *RequestQueue) OnRequestTimeout(hash Uint256) {
	queue.handler.OnRequestError(errors.New("Request timeout with hash: " + hash.String()))
}

func (queue *RequestQueue) OnBlockReceived(block *bloom.MerkleBlock, txIds []*Uint256) error {
	queue.blockReqsLock.Lock()
	defer queue.blockReqsLock.Unlock()

	blockHash := block.Header.Hash()
	// Check if received block is in the request queue
	var ok bool
	var request *Request
	if request, ok = queue.blockRequests[blockHash]; !ok {
		fmt.Println("Unknown block received: ", blockHash.String())
		return nil
	}

	// Remove from block request list
	request.Finish()
	delete(queue.blockRequests, blockHash)
	<-queue.blocksQueue

	// Request block transactions
	queue.StartBlockTxsRequest(request.peer, block, txIds)

	return nil
}

func (queue *RequestQueue) OnTxReceived(tx *Transaction) error {
	queue.blockTxsReqsLock.Lock()
	txId := tx.Hash()
	var ok bool
	var blockHash Uint256
	if blockHash, ok = queue.blockTxs[txId]; !ok {
		fmt.Println("Unknown transaction received: ", txId.String())
		queue.blockTxsReqsLock.Unlock()
		return nil
	}

	// Remove from map
	delete(queue.blockTxs, txId)

	var blockTxsRequest *BlockTxsRequest
	if blockTxsRequest, ok = queue.blockTxsRequests[blockHash]; !ok {
		queue.blockTxsReqsLock.Unlock()
		return errors.New("Request not exist with id: " + blockHash.String())
	}

	finished, err := blockTxsRequest.OnTxReceived(tx)
	if err != nil {
		queue.blockTxsReqsLock.Unlock()
		return err
	}

	if finished {
		delete(queue.blockTxsRequests, blockHash)
		<-queue.blockTxsQueue
		queue.blockTxsReqsLock.Unlock()
		queue.OnRequestFinished(blockTxsRequest)
		return nil
	}
	queue.blockTxsReqsLock.Unlock()
	return nil
}

func (queue *RequestQueue) OnRequestFinished(request *BlockTxsRequest) {
	// Add to finished pool
	queue.finished.Add(request)

	log.Debug("Queue on request finished pool size: ", queue.finished.Length())

	// Callback finish event and pass the finished requests pool
	queue.handler.OnRequestFinished(queue.finished)
}

func (queue *RequestQueue) Clear() {
	// Clear hashes chan
	for len(queue.hashesQueue) > 0 {
		<-queue.hashesQueue
	}
	// Clear block requests chan
	for len(queue.blocksQueue) > 0 {
		<-queue.blocksQueue
	}
	// Clear block txs requests chan
	for len(queue.blockTxsQueue) > 0 {
		<-queue.blockTxsQueue
	}

	// Clear block requests
	queue.blockReqsLock.Lock()
	for hash, request := range queue.blockRequests {
		request.Finish()
		delete(queue.blockRequests, hash)
	}
	queue.blockReqsLock.Unlock()

	// Clear block txs requests
	queue.blockTxsReqsLock.Lock()
	for hash, request := range queue.blockTxsRequests {
		request.Finish()
		delete(queue.blockTxsRequests, hash)
	}
	queue.blockTxsReqsLock.Unlock()

	// Clear finished requests pool
	queue.finished.Clear()
}
