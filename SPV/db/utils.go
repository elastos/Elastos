package db

import (
	. "SPVWallet/core/transaction"
	. "SPVWallet/core"
)

func ToStordTxn(txn *Transaction, height uint32) *Txn {
	tx := new(Txn)
	tx.TxId = *txn.Hash()
	tx.Height = height
	tx.RawData = txn.Bytes()
	return tx
}

func ToStordUTXO(txId *Uint256, height uint32, index int, output *Output) *UTXO {
	utxo := new(UTXO)
	utxo.Op = *NewOutPoint(*txId, uint16(index))
	utxo.Value = output.Value
	utxo.LockTime = output.OutputLock
	utxo.AtHeight = height
	return utxo
}

func InputFromStoreUTXO(utxo *UTXO) *Input {
	input := new(Input)
	input.ReferTxID = utxo.Op.TxID
	input.ReferTxOutputIndex = utxo.Op.Index
	input.Sequence = utxo.LockTime
	return input
}
