// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package chain

import (
	"io"
	"math/rand"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/benchmark/common/tx"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
)

type TxRepository struct {
	params         *GenerationParams
	accountKeys    []common.Uint168
	accounts       map[common.Uint168]*account.Account
	utxos          map[common.Uint168][]types.UTXO
	foundation     *account.Account
	foundationUTXO types.UTXO
}

func (r *TxRepository) Params() *GenerationParams {
	return r.params
}

func (r *TxRepository) GetFoundationAccount() *account.Account {
	return r.foundation
}

func (r *TxRepository) SetFoundationUTXO(utxo *types.UTXO) {
	r.foundationUTXO = *utxo
}

func (r *TxRepository) GeneratePressureTxs(
	height uint32, size int) (txs []*types.Transaction, err error) {
	if height <= r.params.PrepareStartHeight {
		return
	}

	totalTxSize := 0
	var txn *types.Transaction
	txs = make([]*types.Transaction, 0)
	for {
		txn, err = r.generateTx()
		if err != nil {
			return
		}
		txs = append(txs, txn)

		txSize := txn.GetSize()
		if totalTxSize + txSize > size {
			break
		}
		totalTxSize += txSize
	}
	r.updateUTXOs(txs)

	return
}

func (r *TxRepository) GenerateTxs(
	height uint32) (txs []*types.Transaction, err error) {
	if height <= r.params.PrepareStartHeight {
		return
	}

	refCount := r.calculateRefCount(height)

	// tx consume UTXOs
	var txn *types.Transaction
	txs = make([]*types.Transaction, 0, refCount+1)
	for i := uint32(0); i < refCount; i++ {
		txn, err = r.generateTx()
		if err != nil {
			return
		}
		txs = append(txs, txn)
	}
	r.updateUTXOs(txs)

	// tx from foundation
	txn, err = r.allocateFromFoundation(
		r.params.InputsPerBlock - refCount)
	if err != nil {
		return
	}
	txs = append(txs, txn)
	r.updateByAllocateFundTx(txn)

	return
}

func (r *TxRepository) calculateRefCount(height uint32) uint32 {
	refCount := uint32(0)
	if height > r.params.RandomStartHeight {
		count := int64(0)
		if r.params.MaxRefersCount > r.params.MinRefersCount {
			referRange := r.params.MaxRefersCount - r.params.MinRefersCount
			count = rand.Int63n(int64(referRange))
		}
		refCount = r.params.MinRefersCount + uint32(count)
	}
	return refCount
}

func (r *TxRepository) Serialize(w io.Writer) (err error) {
	if err = r.params.Serialize(w); err != nil {
		return
	}

	if err = r.serializeAccount(w, r.foundation); err != nil {
		return
	}

	if err = r.foundationUTXO.Serialize(w); err != nil {
		return
	}

	if err = common.WriteVarUint(w, uint64(len(r.accountKeys))); err != nil {
		return
	}
	for _, v := range r.accountKeys {
		if err = v.Serialize(w); err != nil {
			return
		}
	}

	if err = common.WriteVarUint(w, uint64(len(r.accounts))); err != nil {
		return
	}
	for k, v := range r.accounts {
		if err = k.Serialize(w); err != nil {
			return
		}
		if err = r.serializeAccount(w, v); err != nil {
			return
		}
	}

	if err = common.WriteVarUint(w, uint64(len(r.utxos))); err != nil {
		return
	}
	for k, v := range r.utxos {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = common.WriteVarUint(w, uint64(len(v))); err != nil {
			return
		}
		for _, v := range v {
			if err = v.Serialize(w); err != nil {
				return
			}
		}
	}

	return
}

func (r *TxRepository) Deserialize(reader io.Reader) (err error) {
	r.params = &GenerationParams{}
	if err = r.params.Deserialize(reader); err != nil {
		return
	}

	if r.foundation, err = r.deserializeAccount(reader); err != nil {
		return
	}

	if err = r.foundationUTXO.Deserialize(reader); err != nil {
		return
	}

	var length uint64
	if length, err = common.ReadVarUint(reader, 0); err != nil {
		return
	}
	var key common.Uint168
	for i := uint64(0); i < length; i++ {
		if err = key.Deserialize(reader); err != nil {
			return
		}
		r.accountKeys = append(r.accountKeys, key)
	}

	if length, err = common.ReadVarUint(reader, 0); err != nil {
		return
	}
	var ac *account.Account
	r.accounts = make(map[common.Uint168]*account.Account, length)
	for i := uint64(0); i < length; i++ {
		if err = key.Deserialize(reader); err != nil {
			return
		}
		if ac, err = r.deserializeAccount(reader); err != nil {
			return
		}
		r.accounts[key] = ac
	}

	if length, err = common.ReadVarUint(reader, 0); err != nil {
		return
	}
	var utxo types.UTXO
	var utxoCount uint64
	r.utxos = make(map[common.Uint168][]types.UTXO, length)
	for i := uint64(0); i < length; i++ {
		if err = key.Deserialize(reader); err != nil {
			return
		}

		if utxoCount, err = common.ReadVarUint(reader, 0); err != nil {
			return
		}
		utxos := make([]types.UTXO, 0, utxoCount)
		for j := uint64(0); j < utxoCount; j++ {
			if err = utxo.Deserialize(reader); err != nil {
				return
			}
			utxos = append(utxos, utxo)
		}
		r.utxos[key] = utxos
	}

	return
}

func (r *TxRepository) serializeAccount(w io.Writer, ac *account.Account) error {
	return common.WriteVarBytes(w, ac.PrivateKey)
}

func (r *TxRepository) deserializeAccount(
	reader io.Reader) (ac *account.Account, err error) {
	var priBuf []byte
	if priBuf, err = common.ReadVarBytes(reader, crypto.SignerLength,
		"private key"); err != nil {
		return
	}

	return account.NewAccountWithPrivateKey(priBuf)
}

func (r *TxRepository) updateByAllocateFundTx(allocTx *types.Transaction) {
	r.appendUTXOs(allocTx, len(allocTx.Outputs)-1)

	r.foundationUTXO = types.UTXO{
		TxID:  allocTx.Hash(),
		Index: uint16(len(allocTx.Outputs) - 1),
		Value: allocTx.Outputs[len(allocTx.Outputs)-1].Value,
	}
}

func (r *TxRepository) updateUTXOs(txns []*types.Transaction) {
	for _, txn := range txns {
		r.appendUTXOs(txn, 0)
	}
}

func (r *TxRepository) appendUTXOs(txn *types.Transaction, utxoCount int) {
	for i, o := range txn.Outputs {
		if utxoCount != 0 && i >= utxoCount {
			break
		}

		addr := o.ProgramHash
		utxo := types.UTXO{
			TxID:  txn.Hash(),
			Index: uint16(i),
			Value: o.Value,
		}
		if _, ok := r.utxos[addr]; ok {
			r.utxos[addr] = append(r.utxos[addr], utxo)
		} else {
			r.utxos[addr] = []types.UTXO{utxo}
		}
	}
}

func (r *TxRepository) allocateFromFoundation(inCount uint32) (
	transaction *types.Transaction, err error) {
	accounts := make([]*account.Account, 0, inCount)
	for i := uint32(0); i < inCount; i++ {
		ac := r.randomAccount()
		accounts = append(accounts, ac)
	}
	generator := tx.NewGenerator(types.TransferAsset, accounts...)
	transaction = generator.Generate()

	assigner := tx.NewAssigner(tx.FixAmount, r.foundation, &r.foundationUTXO)
	err = assigner.SignAndChange(transaction)
	return
}

func (r *TxRepository) generateTx() (txn *types.Transaction, err error) {
	outAccount := r.randomAccount()
	// todo generate tx by random tx types later
	generator := tx.NewGenerator(types.TransferAsset, outAccount)
	txn = generator.Generate()

	inAccount := r.randomAccount()
	utxo := r.consumeRandomUTXO(inAccount)
	assigner := tx.NewAssigner(tx.NoChanges, inAccount, &utxo)
	err = assigner.SignAndChange(txn)
	return
}

func (r *TxRepository) randomAccount() *account.Account {
	index := rand.Int63n(int64(r.params.AddressCount))
	return r.accounts[r.accountKeys[index]]
}

func (r *TxRepository) consumeRandomUTXO(ac *account.Account) types.UTXO {
	utxos := r.utxos[ac.ProgramHash]
	index := rand.Int63n(int64(len(utxos)))
	result := utxos[index]

	utxos[index] = utxos[len(utxos)-1]
	utxos = utxos[:len(utxos)-1]
	r.utxos[ac.ProgramHash] = utxos

	return result
}

func NewTxRepository(params *GenerationParams) (result *TxRepository,
	err error) {
	result = &TxRepository{
		params:      params,
		accountKeys: []common.Uint168{},
		accounts:    map[common.Uint168]*account.Account{},
		utxos:       map[common.Uint168][]types.UTXO{},
	}

	var ac *account.Account
	if ac, err = account.NewAccount(); err != nil {
		return
	}
	result.foundation = ac

	for i := uint32(0); i < params.AddressCount; i++ {
		if ac, err = account.NewAccount(); err != nil {
			return
		}
		result.accounts[ac.ProgramHash] = ac
		result.accountKeys = append(result.accountKeys, ac.ProgramHash)
	}

	return
}
