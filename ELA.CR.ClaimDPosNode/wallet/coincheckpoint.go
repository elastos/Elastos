package wallet

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	"github.com/elastos/Elastos.ELA/core/types"
)

type CoinsCheckPoint struct {
	height uint32
	coins  map[types.OutPoint]*Coin
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
	return nil
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

	return nil
}

func (ccp *CoinsCheckPoint) Key() string {
	return key
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
	log.Error(err.Error())
}

func (ccp *CoinsCheckPoint) OnBlockSaved(block *types.DposBlock) {
	for _, tx := range block.Transactions {
		// remove the spent coins
		for _, input := range tx.Inputs {
			_, exist := ccp.coins[input.Previous]
			if exist {
				delete(ccp.coins, input.Previous)
			}
		}

		// add the new coins
		for index, output := range tx.Outputs {
			addr, err := output.ProgramHash.ToAddress()
			if err != nil {
				panic("invalid output program hash")
			}
			_, exist := Wal.addressBook[addr]
			if exist {
				op := types.OutPoint{
					TxID:  tx.Hash(),
					Index: uint16(index),
				}
				ccp.coins[op] = &Coin{
					TxVersion: tx.Version,
					Output:    output,
					Height:    block.Height,
				}
			}
		}
	}
}

func (ccp *CoinsCheckPoint) OnRollbackTo(height uint32) error {
	bestHeight := Store.GetHeight()
	if height >= bestHeight {
		return nil
	}
	for i := bestHeight; i == height; i-- {
		hash, err := Store.GetBlockHash(height)
		if err != nil {
			return err
		}
		block, err := Store.GetBlock(hash)
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
				_, exist := ccp.coins[op]
				if exist {
					delete(ccp.coins, op)
				}
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
				_, exist := Wal.addressBook[addr]
				if exist {
					ccp.coins[input.Previous] = &Coin{
						TxVersion: tx.Version,
						Output:    output,
						Height:    i,
					}
				}
			}
		}
	}

	return nil
}

func (ccp *CoinsCheckPoint) getWalletUnspent(addresses []string) (map[types.OutPoint]*Coin, error) {
	addressMap := make(map[string]struct{})
	for _, addr := range addresses {
		addressMap[addr] = struct{}{}
	}

	coins := make(map[types.OutPoint]*Coin, 0)
	for op, coin := range ccp.coins {
		coinAddr, err := coin.Output.ProgramHash.ToAddress()
		if err != nil {
			return nil, err
		}
		_, exist := addressMap[coinAddr]
		if exist {
			coins[op] = coin
		}
	}

	return coins, nil
}

func (ccp *CoinsCheckPoint) ListUnspent(address string, enableUtxoDB bool) (map[common.Uint256][]*blockchain.UTXO,
	error) {
	if enableUtxoDB {
		programHash, err := common.Uint168FromAddress(address)
		if err != nil {
			return nil, err
		}
		unspents, err := Store.GetUnspentsFromProgramHash(*programHash)
		if err != nil {
			return nil, err
		}

		return unspents, nil
	}

	coins, err := ccp.getWalletUnspent([]string{address})
	if err != nil {
		return nil, err
	}
	utxos := make([]*blockchain.UTXO, 0)
	for op, coin := range coins {
		utxos = append(utxos, &blockchain.UTXO{
			TxID:  op.TxID,
			Index: uint32(op.Index),
			Value: coin.Output.Value,
		})
	}
	unspents := make(map[common.Uint256][]*blockchain.UTXO, 0)
	unspents[*account.SystemAssetID] = utxos

	return unspents, nil
}

func (ccp *CoinsCheckPoint) RescanWallet() error {
	bestHeight := Store.GetHeight()
	for i := uint32(0); i <= bestHeight; i++ {
		hash, err := Store.GetBlockHash(i)
		if err != nil {
			return err
		}
		block, err := Store.GetBlock(hash)
		if err != nil {
			return err
		}
		ccp.OnBlockSaved(&types.DposBlock{
			Block: block,
		})
	}

	return nil
}

func NewCoinCheckPoint() *CoinsCheckPoint {
	return &CoinsCheckPoint{
		height: 0,
		coins:  make(map[types.OutPoint]*Coin, 0),
	}
}
