package db

import (
	"bytes"

	. "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	. "github.com/elastos/Elastos.ELA.SPV/core"
)

func ToStoredTxn(tx Transaction, height uint32) *Txn {
	txn := new(Txn)
	txn.TxId = *tx.Hash()
	txn.Height = height
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	txn.RawData = buf.Bytes()
	return txn
}

func ToStoredUTXO(txId *Uint256, height uint32, index int, value Fixed64, lockTime uint32) *UTXO {
	utxo := new(UTXO)
	utxo.Op = *NewOutPoint(*txId, uint16(index))
	utxo.Value = value
	utxo.LockTime = lockTime
	utxo.AtHeight = height
	return utxo
}

func InputFromStoredUTXO(utxo *UTXO) *Input {
	input := new(Input)
	input.ReferTxID = utxo.Op.TxID
	input.ReferTxOutputIndex = utxo.Op.Index
	input.Sequence = utxo.LockTime
	return input
}
