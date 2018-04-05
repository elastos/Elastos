package spvwallet

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
)

type BlockTxsRequest struct {
	sync.Mutex
	block          bloom.MerkleBlock
	txRequestQueue map[Uint256]*Request
	receivedTxs    []tx.Transaction
}

func (req *BlockTxsRequest) Finish() {
	// Finish transaction requests
	for hash, request := range req.txRequestQueue {
		request.Finish()
		delete(req.txRequestQueue, hash)
	}
}

func (req *BlockTxsRequest) OnTxReceived(tx *tx.Transaction) (bool, error) {
	req.Lock()
	defer req.Unlock()

	txId := *tx.Hash()
	var ok bool
	var txRequest *Request
	if txRequest, ok = req.txRequestQueue[txId]; !ok {
		return false, errors.New("Received transaction not belong to block: " +
			req.block.BlockHeader.Hash().String() + ", tx: " + tx.Hash().String())
	}

	// Remove from map
	txRequest.Finish()
	delete(req.txRequestQueue, txId)

	req.receivedTxs = append(req.receivedTxs, *tx)

	return len(req.txRequestQueue) == 0, nil
}
