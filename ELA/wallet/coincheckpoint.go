// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"bytes"
	"io"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
)

// CoinsCheckPoint implement the ICheckPoint interface and store all coins
// which be subscribed.
type CoinsCheckPoint struct {
	height     uint32
	coins      map[types.OutPoint]*Coin
	ownedCoins OwnedCoins

	sync.RWMutex
}

func (ccp *CoinsCheckPoint) StartHeight() uint32 {
	return 0
}

func (ccp *CoinsCheckPoint) Priority() checkpoint.Priority {
	return checkpoint.High
}

func (ccp *CoinsCheckPoint) OnInit() {
}

func (ccp *CoinsCheckPoint) Serialize(w io.Writer) error {
	if err := common.WriteUint32(w, ccp.height); err != nil {
		return err
	}
	if err := common.WriteUint32(w, uint32(len(ccp.coins))); err != nil {
		return err
	}
	for k, v := range ccp.coins {
		if err := k.Serialize(w); err != nil {
			return err
		}
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	return ccp.ownedCoins.Serialize(w)
}

func (ccp *CoinsCheckPoint) Deserialize(r io.Reader) error {
	height, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	ccp.height = height

	count, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	for i := uint32(0); i < count; i++ {
		var op types.OutPoint
		if err := op.Deserialize(r); err != nil {
			return err
		}
		coin := new(Coin)
		if err := coin.Deserialize(r); err != nil {
			return err
		}
		ccp.coins[op] = coin
	}

	return ccp.ownedCoins.Deserialize(r)
}

func (ccp *CoinsCheckPoint) Key() string {
	return utxoCheckPointKey
}

func (ccp *CoinsCheckPoint) Snapshot() checkpoint.ICheckPoint {
	buf := new(bytes.Buffer)
	ccp.Serialize(buf)
	newCoinCheckPoint := NewCoinCheckPoint()
	newCoinCheckPoint.Deserialize(buf)

	return newCoinCheckPoint
}

func (ccp *CoinsCheckPoint) GetHeight() uint32 {
	return ccp.height
}

func (ccp *CoinsCheckPoint) SetHeight(height uint32) {
	ccp.height = height
}

func (ccp *CoinsCheckPoint) SavePeriod() uint32 {
	return savePeriod
}

func (ccp *CoinsCheckPoint) EffectivePeriod() uint32 {
	return effectivePeriod
}

func (ccp *CoinsCheckPoint) DataExtension() string {
	return dataExtension
}

func (ccp *CoinsCheckPoint) Generator() func(buf []byte) checkpoint.ICheckPoint {
	return func(buf []byte) checkpoint.ICheckPoint {
		newBuf := bytes.NewBuffer(buf)
		ccp := NewCoinCheckPoint()
		ccp.Deserialize(newBuf)

		return ccp
	}
}

func (ccp *CoinsCheckPoint) LogError(err error) {
	log.Warn(err.Error())
}

func (ccp *CoinsCheckPoint) OnBlockSaved(block *types.DposBlock) {
	ccp.Lock()
	defer ccp.Unlock()

	for _, tx := range block.Transactions {
		// remove the spent coins
		for _, input := range tx.Inputs {
			_, exist := ccp.coins[input.Previous]
			if exist {
				ccp.removeCoin(&input.Previous)
			}
		}

		// add the new coins
		for index, output := range tx.Outputs {
			op := types.OutPoint{
				TxID:  tx.Hash(),
				Index: uint16(index),
			}
			ccp.appendCoin(&op, &Coin{
				TxVersion: tx.Version,
				Output:    output,
				Height:    block.Height,
			})
		}
	}
}

func (ccp *CoinsCheckPoint) OnRollbackTo(height uint32) error {
	ccp.Lock()
	defer ccp.Unlock()

	bestHeight := Chain.GetHeight()
	if height >= bestHeight {
		return nil
	}
	for i := bestHeight; i == height; i-- {
		hash, err := Chain.GetBlockHash(height)
		if err != nil {
			return err
		}
		block, err := Store.GetFFLDB().GetBlock(hash)
		if err != nil {
			return err
		}
		for _, tx := range block.Transactions {
			// rollback coins from output
			for index := range tx.Outputs {
				op := types.OutPoint{
					TxID:  tx.Hash(),
					Index: uint16(index),
				}
				ccp.removeCoin(&op)
			}
			// recover coins from input
			reference, err := Store.GetTxReference(tx)
			if err != nil {
				return err
			}
			for input, output := range reference {
				addr, err := output.ProgramHash.ToAddress()
				if err != nil {
					return err
				}
				_, exist := GetWalletAccount(addr)
				if exist {
					ccp.appendCoin(&input.Previous, &Coin{
						TxVersion: tx.Version,
						Output:    output,
						Height:    i,
					})
				}
			}
		}
	}

	return nil
}

func (ccp *CoinsCheckPoint) appendCoin(op *types.OutPoint, coin *Coin) error {
	addr, err := coin.Output.ProgramHash.ToAddress()
	if err != nil {
		return err
	}

	// append wallet coin, vote utxo and deposit coin
	_, exist := GetWalletAccount(addr)
	if exist || coin.Output.Type == types.OTVote ||
		contract.GetPrefixType(coin.Output.ProgramHash) == contract.PrefixDeposit {
		ccp.coins[*op] = coin
		ccp.ownedCoins.append(addr, op)
	}

	return nil
}

func (ccp *CoinsCheckPoint) removeCoin(op *types.OutPoint) {
	coin, exist := ccp.coins[*op]
	if !exist {
		return
	}
	delete(ccp.coins, *op)
	addr, err := coin.Output.ProgramHash.ToAddress()
	if err != nil {
		panic("invalid coin in wallet")
	}
	ccp.ownedCoins.remove(addr, op)
}

func (ccp *CoinsCheckPoint) ListCoins(owner string) map[types.OutPoint]*Coin {
	ccp.RLock()
	defer ccp.RUnlock()

	coins := make(map[types.OutPoint]*Coin, 0)
	ops := ccp.ownedCoins.list(owner)
	for _, op := range ops {
		coin := ccp.coins[*op]
		coins[*op] = coin
	}

	return coins
}

func (ccp *CoinsCheckPoint) AppendCoin(owner string, op *types.OutPoint, coin *Coin) {
	ccp.Lock()
	defer ccp.Unlock()

	ccp.coins[*op] = coin
	ccp.ownedCoins.append(owner, op)
}

func (ccp *CoinsCheckPoint) GetCoin(owner string, op *types.OutPoint) (*Coin, bool) {
	ccp.RLock()
	defer ccp.RUnlock()

	if op == nil {
		op = &types.OutPoint{}
	}
	_, exist := ccp.ownedCoins[CoinOwnership{owner, *op}]
	if !exist {
		return nil, false
	}
	coin, exist := ccp.coins[*op]

	return coin, exist
}

func NewCoinCheckPoint() *CoinsCheckPoint {
	return &CoinsCheckPoint{
		height:     0,
		coins:      make(map[types.OutPoint]*Coin, 0),
		ownedCoins: NewOwnedCoins(),
	}
}
