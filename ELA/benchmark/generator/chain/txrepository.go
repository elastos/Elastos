package chain

import (
	"math/rand"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/benchmark/generator/tx"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type TxRepository struct {
	params         *GenerationParams
	accountKeys    []common.Uint168
	accounts       map[common.Uint168]*account.Account
	utxos          map[common.Uint168][]types.UTXO
	foundation     *account.Account
	foundationUTXO types.UTXO
}

func (r *TxRepository) GetFoundationAccount() *account.Account {
	return r.foundation
}

func (r *TxRepository) SetFoundationUTXO(utxo *types.UTXO) {
	r.foundationUTXO = *utxo
}

func (r *TxRepository) GenerateTxs(
	height uint32) (txs []*types.Transaction, err error) {
	if height <= r.params.PrepareStartHeight {
		return
	}

	refCount := uint32(0)
	if height > r.params.RandomStartHeight {
		count := int64(0)
		if r.params.MaxRefersCount > r.params.MinRefersCount {
			referRange := r.params.MaxRefersCount - r.params.MinRefersCount
			count = rand.Int63n(int64(referRange))
		}
		refCount = r.params.MinRefersCount + uint32(count)
	}

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
