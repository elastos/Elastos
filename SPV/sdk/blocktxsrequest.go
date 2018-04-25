package sdk

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockTxsRequest struct {
	sync.Mutex
	BlockHash      Uint256
	Block          bloom.MerkleBlock
	txRequestQueue map[Uint256]*Request
	Txs            []Transaction
}

func (req *BlockTxsRequest) Finish() {
	// Finish transaction requests
	for hash, request := range req.txRequestQueue {
		request.Finish()
		delete(req.txRequestQueue, hash)
	}
}

func (req *BlockTxsRequest) OnTxReceived(tx *Transaction) (bool, error) {
	req.Lock()
	defer req.Unlock()

	txId := tx.Hash()
	var ok bool
	var txRequest *Request
	if txRequest, ok = req.txRequestQueue[txId]; !ok {
		return false, errors.New("Received transaction not belong to block: " +
			req.Block.Header.Hash().String() + ", tx: " + tx.Hash().String())
	}

	// Remove from map
	txRequest.Finish()
	delete(req.txRequestQueue, txId)

	req.Txs = append(req.Txs, *tx)

	return len(req.txRequestQueue) == 0, nil
}
