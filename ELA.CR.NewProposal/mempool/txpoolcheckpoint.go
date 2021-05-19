// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/elanet/pact"
)

const (
	// checkpointKey defines key of DPoS checkpoint.
	checkpointKey = "txPool"

	// checkpointExtension defines checkpoint file extension of DPoS checkpoint.
	checkpointExtension = ".txpcp"

	// checkpointHeight defines interval height between two neighbor check
	// points.
	checkpointHeight = uint32(1)
)

type txPoolCheckpoint struct {
	// transaction which have been verified will put into this map
	txnList map[common.Uint256]*types.Transaction
	txFees  *txFeeOrderedList
	txPool  *TxPool

	height              uint32
	initConflictManager func(map[common.Uint256]*types.Transaction)
}

func (c *txPoolCheckpoint) OnBlockSaved(*types.DposBlock) {
}

func (c *txPoolCheckpoint) OnRollbackTo(uint32) error {
	return nil
}

func (c *txPoolCheckpoint) Key() string {
	return checkpointKey
}

func (c *txPoolCheckpoint) Snapshot() checkpoint.ICheckPoint {
	c.txPool.Lock()
	defer c.txPool.Unlock()

	buf := bytes.Buffer{}
	if err := c.Serialize(&buf); err != nil {
		c.LogError(err)
		return nil
	}
	result := newTxPoolCheckpoint(c.txPool, c.initConflictManager)
	if err := result.Deserialize(&buf); err != nil {
		c.LogError(err)
		return nil
	}
	return result
}

func (c *txPoolCheckpoint) GetHeight() uint32 {
	return c.height
}

func (c *txPoolCheckpoint) SetHeight(height uint32) {
	c.height = height
}

func (c *txPoolCheckpoint) SavePeriod() uint32 {
	return checkpointHeight
}

func (c *txPoolCheckpoint) EffectivePeriod() uint32 {
	return checkpointHeight
}

func (c *txPoolCheckpoint) DataExtension() string {
	return checkpointExtension
}

func (c *txPoolCheckpoint) Generator() func(buf []byte) checkpoint.ICheckPoint {
	return func(buf []byte) checkpoint.ICheckPoint {
		stream := bytes.Buffer{}
		stream.Write(buf)

		result := newTxPoolCheckpoint(c.txPool, c.initConflictManager)
		if err := result.Deserialize(&stream); err != nil {
			c.LogError(err)
			return nil
		}
		return result
	}
}

func (c *txPoolCheckpoint) LogError(err error) {
	log.Warn(err)
}

func (c *txPoolCheckpoint) Priority() checkpoint.Priority {
	return checkpoint.Low
}

func (c *txPoolCheckpoint) OnInit() {
	c.initConflictManager(c.txnList)
}

func (c *txPoolCheckpoint) StartHeight() uint32 {
	return uint32(1)
}

func (c *txPoolCheckpoint) Serialize(w io.Writer) (err error) {
	if err = common.WriteUint32(w, c.height); err != nil {
		return
	}
	if err = common.WriteVarUint(w, uint64(len(c.txnList))); err != nil {
		return
	}
	for k, v := range c.txnList {
		if err = k.Serialize(w); err != nil {
			return
		}
		if err = v.Serialize(w); err != nil {
			return
		}
	}

	return c.txFees.Serialize(w)
}

func (c *txPoolCheckpoint) Deserialize(r io.Reader) (err error) {
	if c.height, err = common.ReadUint32(r); err != nil {
		return
	}
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	var hash common.Uint256
	for i := uint64(0); i < count; i++ {
		tx := &types.Transaction{}
		if err = hash.Deserialize(r); err != nil {
			return
		}
		if err = tx.Deserialize(r); err != nil {
			return
		}
		c.txPool.appendToTxPool(tx)
	}

	c.txFees = newTxFeeOrderedList(c.txPool.onPopBack, pact.MaxTxPoolSize)
	return c.txFees.Deserialize(r)
}

func newTxPoolCheckpoint(txPool *TxPool, initConflictManager func(
	map[common.Uint256]*types.Transaction)) *txPoolCheckpoint {
	return &txPoolCheckpoint{
		txPool:              txPool,
		txnList:             map[common.Uint256]*types.Transaction{},
		txFees:              newTxFeeOrderedList(txPool.onPopBack, pact.MaxTxPoolSize),
		height:              0,
		initConflictManager: initConflictManager,
	}
}
