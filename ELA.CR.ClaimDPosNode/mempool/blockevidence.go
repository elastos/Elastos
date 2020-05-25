// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package mempool

import (
	"bytes"
	"errors"
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/events"
)

func (bm *BlockPool) generateBlockEvidence(block *types.Block, confirm *payload.Confirm) (
	*payload.BlockEvidence, uint32, error) {
	headerBuf := new(bytes.Buffer)
	if err := block.Header.Serialize(headerBuf); err != nil {
		return nil, 0, err
	}

	confirmBuf := new(bytes.Buffer)
	if err := confirm.Serialize(confirmBuf); err != nil {
		return nil, 0, err
	}
	confirmSigners, err := bm.getConfirmSigners(confirm)
	if err != nil {
		return nil, 0, err
	}

	return &payload.BlockEvidence{
		Header:       headerBuf.Bytes(),
		BlockConfirm: confirmBuf.Bytes(),
		Signers:      confirmSigners,
	}, confirm.Proposal.ViewOffset, nil
}

func (bm *BlockPool) getConfirmSigners(confirm *payload.Confirm) ([][]byte, error) {

	var signers []string
	for _, v := range confirm.Votes {
		signers = append(signers, common.BytesToHexString(v.Signer))
	}
	sort.Strings(signers)

	result := make([][]byte, 0)
	for _, v := range signers {
		signer, err := common.HexStringToBytes(v)
		if err != nil {
			return nil, err
		}

		result = append(result, signer)
	}

	return result, nil
}

func (bm *BlockPool) CheckConfirmedBlockOnFork(height uint32, block *types.Block) error {
	// main version >= H2
	if height >= bm.chainParams.PublicDPOSHeight {
		blockNode := bm.Chain.GetBlockNode(block.Height)
		if blockNode == nil {
			return errors.New(fmt.Sprintf("no block at height %d exists", height))
		}

		anotherBlock, err := bm.Store.GetFFLDB().GetBlock(*blockNode.Hash)
		if err != nil {
			return err
		}

		if block.Hash().IsEqual(anotherBlock.Hash()) {
			return nil
		}

		confirm, ok := bm.confirms[block.Hash()]
		if !ok {
			return nil
		}

		evidence, offset, err := bm.generateBlockEvidence(block, confirm)
		if err != nil {
			return err
		}

		compareEvidence, compareOffset, err := bm.generateBlockEvidence(
			anotherBlock.Block, anotherBlock.Confirm)
		if err != nil {
			return err
		}

		// IllegalBlockEvidence tx shall not be created if view offset of two block
		// confirms is not equal.
		if offset > compareOffset {
			// do nothing if view offset of block on chain is less than current
			// block
			return nil
		} else if offset < compareOffset &&
			block.Hash().IsEqual(anotherBlock.Hash()) {
			// reorganize chain if view offset of block on chain is more than
			// current block, and these two blocks should be different
			return bm.Chain.ReorganizeChain(block)
		}

		illegalBlocks := &payload.DPOSIllegalBlocks{
			CoinType:    payload.ELACoin,
			BlockHeight: block.Height,
		}

		asc := true
		if bytes.Compare(evidence.Header, compareEvidence.Header) > 0 {
			asc = false
		}

		if asc {
			illegalBlocks.Evidence = *evidence
			illegalBlocks.CompareEvidence = *compareEvidence
		} else {
			illegalBlocks.Evidence = *compareEvidence
			illegalBlocks.CompareEvidence = *evidence
		}

		if err := blockchain.CheckDPOSIllegalBlocks(illegalBlocks); err != nil {
			return err
		}

		tx := &types.Transaction{
			Version:        types.TxVersion09,
			TxType:         types.IllegalBlockEvidence,
			PayloadVersion: payload.IllegalBlockVersion,
			Payload:        illegalBlocks,
			Attributes:     []*types.Attribute{},
			LockTime:       0,
			Programs:       []*program.Program{},
			Outputs:        []*types.Output{},
			Inputs:         []*types.Input{},
			Fee:            0,
		}

		events.Notify(events.ETIllegalBlockEvidence, tx)

		return nil
	}

	// version [0, H2)
	return nil
}
