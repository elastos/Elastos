// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/types"
)

// Hash array related functions
func addRechargeToSideChainTransactionHash(
	chain *blockchain.BlockChain, tx *types.Transaction) (interface{}, error) {
	rechargePayload, ok := tx.Payload.(*types.PayloadRechargeToSideChain)
	if !ok {
		return nil, errors.New("convert the payload of recharge tx failed")
	}

	hash, err := rechargePayload.GetMainchainTxHash(tx.PayloadVersion)
	if err != nil {
		return nil, err
	}
	return *hash, nil
}

// Str array related functions
func strArrayTxReferences(chain *blockchain.BlockChain, tx *types.Transaction) (interface{}, error) {
	reference, err := chain.GetTxReference(tx)
	if err != nil {
		return nil, err
	}

	result := make([]string, 0, len(reference))
	for k := range reference {
		result = append(result, k.ReferKey())
	}
	return result, nil
}
