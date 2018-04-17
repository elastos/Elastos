package chainstore

import (
	"bytes"
	"errors"
	"fmt"

	. "Elastos.ELA/common"
	"Elastos.ELA/common/serialize"
	. "Elastos.ELA/core/ledger"
	tx "Elastos.ELA/core/transaction"
	"Elastos.ELA/core/transaction/payload"
)

func (db *ChainStore) BatchInit() error {
	return db.NewBatch()
}

func (db *ChainStore) BatchFinish() error {
	return db.BatchCommit()
}

// key: DATA_Header || block hash
// value: sysfee(8bytes) || trimmed block
func (db *ChainStore) PersistTrimmedBlock(b *Block) error {
	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(DATA_Header))
	blockHash := b.Hash()
	blockHash.Serialize(key)

	value := bytes.NewBuffer(nil)
	var sysfee uint64 = 0x0000000000000000
	serialize.WriteUint64(value, sysfee)
	b.Trim(value)

	if err := db.BatchPut(key.Bytes(), value.Bytes()); err != nil {
		return err
	}

	return nil
}

func (db *ChainStore) RollbackTrimemedBlock(b *Block) error {
	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(DATA_Header))
	blockHash := b.Hash()
	blockHash.Serialize(key)

	if err := db.BatchDelete(key.Bytes()); err != nil {
		return err
	}

	return nil
}

// key: DATA_BlockHash || height
// value: block hash
func (db *ChainStore) PersistBlockHash(b *Block) error {
	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(DATA_BlockHash))
	if err := serialize.WriteUint32(key, b.Header.Height); err != nil {
		return err
	}

	value := bytes.NewBuffer(nil)
	hashValue := b.Header.Hash()
	hashValue.Serialize(value)

	if err := db.BatchPut(key.Bytes(), value.Bytes()); err != nil {
		return err
	}

	return nil
}

func (db *ChainStore) RollbackBlockHash(b *Block) error {
	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(DATA_BlockHash))
	if err := serialize.WriteUint32(key, b.Header.Height); err != nil {
		return err
	}

	if err := db.BatchDelete(key.Bytes()); err != nil {
		return err
	}

	return nil
}

// key: SYS_CurrentBlock
// value: current block hash || height
func (db *ChainStore) PersistCurrentBlock(b *Block) error {

	currentBlockKey := bytes.NewBuffer(nil)
	currentBlockKey.WriteByte(byte(SYS_CurrentBlock))

	currentBlock := bytes.NewBuffer(nil)
	blockHash := b.Hash()
	blockHash.Serialize(currentBlock)
	serialize.WriteUint32(currentBlock, b.Header.Height)

	if err := db.BatchPut(currentBlockKey.Bytes(), currentBlock.Bytes()); err != nil {
		return err
	}

	return nil
}

func (db *ChainStore) RollbackCurrentBlock(b *Block) error {
	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(SYS_CurrentBlock))

	value := bytes.NewBuffer(nil)
	blockHash := b.Header.Previous
	blockHash.Serialize(value)
	serialize.WriteUint32(value, b.Header.Height-1)

	if err := db.BatchPut(key.Bytes(), value.Bytes()); err != nil {
		return err
	}

	return nil
}

func (db *ChainStore) PersistUnspendUTXOs(b *Block) error {
	unspendUTXOs := make(map[Uint168]map[Uint256]map[uint32][]*tx.UTXOUnspent)
	curHeight := b.Header.Height

	for _, txn := range b.Transactions {
		if txn.TxType == tx.RegisterAsset {
			continue
		}

		for index, output := range txn.Outputs {
			programHash := output.ProgramHash
			assetID := output.AssetID
			value := output.Value

			if _, ok := unspendUTXOs[programHash]; !ok {
				unspendUTXOs[programHash] = make(map[Uint256]map[uint32][]*tx.UTXOUnspent)
			}

			if _, ok := unspendUTXOs[programHash][assetID]; !ok {
				unspendUTXOs[programHash][assetID] = make(map[uint32][]*tx.UTXOUnspent, 0)
			}

			if _, ok := unspendUTXOs[programHash][assetID][curHeight]; !ok {
				var err error
				unspendUTXOs[programHash][assetID][curHeight], err = db.GetUnspentElementFromProgramHash(programHash, assetID, curHeight)
				if err != nil {
					unspendUTXOs[programHash][assetID][curHeight] = make([]*tx.UTXOUnspent, 0)
				}

			}

			u := tx.UTXOUnspent{
				Txid:  txn.Hash(),
				Index: uint32(index),
				Value: value,
			}
			unspendUTXOs[programHash][assetID][curHeight] = append(unspendUTXOs[programHash][assetID][curHeight], &u)
		}

		if !txn.IsCoinBaseTx() {
			for _, input := range txn.UTXOInputs {
				referTxn, height, err := db.GetTransaction(input.ReferTxID)
				if err != nil {
					return err
				}
				index := input.ReferTxOutputIndex
				referTxnOutput := referTxn.Outputs[index]
				programHash := referTxnOutput.ProgramHash
				assetID := referTxnOutput.AssetID

				if _, ok := unspendUTXOs[programHash]; !ok {
					unspendUTXOs[programHash] = make(map[Uint256]map[uint32][]*tx.UTXOUnspent)
				}
				if _, ok := unspendUTXOs[programHash][assetID]; !ok {
					unspendUTXOs[programHash][assetID] = make(map[uint32][]*tx.UTXOUnspent)
				}

				if _, ok := unspendUTXOs[programHash][assetID][height]; !ok {
					unspendUTXOs[programHash][assetID][height], err = db.GetUnspentElementFromProgramHash(programHash, assetID, height)

					if err != nil {
						return errors.New(fmt.Sprintf("[persist] utxoUnspents programHash:%v, assetId:%v height:%v has no unspent UTXO.", programHash, assetID, height))
					}
				}

				flag := false
				listnum := len(unspendUTXOs[programHash][assetID][height])
				for i := 0; i < listnum; i++ {
					if unspendUTXOs[programHash][assetID][height][i].Txid.IsEqual(referTxn.Hash()) && unspendUTXOs[programHash][assetID][height][i].Index == uint32(index) {
						unspendUTXOs[programHash][assetID][height][i] = unspendUTXOs[programHash][assetID][height][listnum-1]
						unspendUTXOs[programHash][assetID][height] = unspendUTXOs[programHash][assetID][height][:listnum-1]
						flag = true
						break
					}
				}
				if !flag {
					return errors.New(fmt.Sprintf("[persist] utxoUnspents NOT find UTXO by txid: %x, index: %d.", referTxn.Hash(), index))
				}
			}
		}
	}

	// batch put the utxoUnspents
	for programHash, programHash_value := range unspendUTXOs {
		for assetId, unspents := range programHash_value {
			for height, unspent := range unspents {
				err := db.PersistUnspentWithProgramHash(programHash, assetId, height, unspent)
				if err != nil {
					return err
				}
			}

		}
	}

	return nil
}

func (db *ChainStore) RollbackUnspendUTXOs(b *Block) error {
	unspendUTXOs := make(map[Uint168]map[Uint256]map[uint32][]*tx.UTXOUnspent)
	height := b.Header.Height
	for _, txn := range b.Transactions {
		if txn.TxType == tx.RegisterAsset {
			continue
		}
		for index, output := range txn.Outputs {
			programHash := output.ProgramHash
			assetID := output.AssetID
			value := output.Value
			if _, ok := unspendUTXOs[programHash]; !ok {
				unspendUTXOs[programHash] = make(map[Uint256]map[uint32][]*tx.UTXOUnspent)
			}
			if _, ok := unspendUTXOs[programHash][assetID]; !ok {
				unspendUTXOs[programHash][assetID] = make(map[uint32][]*tx.UTXOUnspent)
			}
			if _, ok := unspendUTXOs[programHash][assetID][height]; !ok {
				var err error
				unspendUTXOs[programHash][assetID][height], err = db.GetUnspentElementFromProgramHash(programHash, assetID, height)
				if err != nil {
					return errors.New(fmt.Sprintf("[persist] utxoUnspents programHash:%v, assetId:%v has no unspent UTXO.", programHash, assetID))
				}
			}
			u := tx.UTXOUnspent{
				Txid:  txn.Hash(),
				Index: uint32(index),
				Value: value,
			}
			var position int
			for i, unspend := range unspendUTXOs[programHash][assetID][height] {
				if unspend.Txid == u.Txid && unspend.Index == u.Index {
					position = i
					break
				}
			}
			unspendUTXOs[programHash][assetID][height] = append(unspendUTXOs[programHash][assetID][height][:position], unspendUTXOs[programHash][assetID][height][position+1:]...)
		}

		if !txn.IsCoinBaseTx() {
			for _, input := range txn.UTXOInputs {
				referTxn, hh, err := db.GetTransaction(input.ReferTxID)
				if err != nil {
					return err
				}
				index := input.ReferTxOutputIndex
				referTxnOutput := referTxn.Outputs[index]
				programHash := referTxnOutput.ProgramHash
				assetID := referTxnOutput.AssetID
				if _, ok := unspendUTXOs[programHash]; !ok {
					unspendUTXOs[programHash] = make(map[Uint256]map[uint32][]*tx.UTXOUnspent)
				}
				if _, ok := unspendUTXOs[programHash][assetID]; !ok {
					unspendUTXOs[programHash][assetID] = make(map[uint32][]*tx.UTXOUnspent)
				}
				if _, ok := unspendUTXOs[programHash][assetID][hh]; !ok {
					unspendUTXOs[programHash][assetID][hh], err = db.GetUnspentElementFromProgramHash(programHash, assetID, hh)
					if err != nil {
						unspendUTXOs[programHash][assetID][hh] = make([]*tx.UTXOUnspent, 0)
					}
				}
				u := tx.UTXOUnspent{
					Txid:  referTxn.Hash(),
					Index: uint32(index),
					Value: referTxnOutput.Value,
				}
				unspendUTXOs[programHash][assetID][hh] = append(unspendUTXOs[programHash][assetID][hh], &u)
			}
		}
	}

	for programHash, programHash_value := range unspendUTXOs {
		for assetId, unspents := range programHash_value {
			for height, unspent := range unspents {
				err := db.PersistUnspentWithProgramHash(programHash, assetId, height, unspent)
				if err != nil {
					return err
				}
			}

		}
	}

	return nil
}

func (db *ChainStore) PersistTransactions(b *Block) error {

	for _, txn := range b.Transactions {
		if err := db.PersistTransaction(txn, b.Header.Height); err != nil {
			return err
		}
		if txn.TxType == tx.RegisterAsset {
			regPayload := txn.Payload.(*payload.RegisterAsset)
			if err := db.PersistAsset(txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}
	}
	return nil
}

func (db *ChainStore) RollbackTransactions(b *Block) error {
	for _, txn := range b.Transactions {
		if err := db.RollbackTransaction(txn); err != nil {
			return err
		}
		if txn.TxType == tx.RegisterAsset {
			if err := db.RollbackAsset(txn.Hash()); err != nil {
				return err
			}
		}
	}

	return nil
}

func (db *ChainStore) RollbackTransaction(txn *tx.Transaction) error {

	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(DATA_Transaction))
	txnHash := txn.Hash()
	txnHash.Serialize(key)

	if err := db.BatchDelete(key.Bytes()); err != nil {
		return err
	}

	return nil
}

func (bd *ChainStore) RollbackAsset(assetId Uint256) error {
	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(ST_Info))
	assetId.Serialize(key)

	if err := bd.BatchDelete(key.Bytes()); err != nil {
		return err
	}

	return nil
}

func (db *ChainStore) PersistUnspend(b *Block) error {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspents := make(map[Uint256][]uint16)
	for _, txn := range b.Transactions {
		if txn.TxType == tx.RegisterAsset {
			continue
		}
		txnHash := txn.Hash()
		for index := range txn.Outputs {
			unspents[txnHash] = append(unspents[txnHash], uint16(index))
		}
		if !txn.IsCoinBaseTx() {
			for index, input := range txn.UTXOInputs {
				referTxnHash := input.ReferTxID
				if _, ok := unspents[referTxnHash]; !ok {
					unspentValue, err := db.Get(append(unspentPrefix, referTxnHash.Bytes()...))
					if err != nil {
						return err
					}
					unspents[referTxnHash], err = GetUint16Array(unspentValue)
					if err != nil {
						return err
					}
				}

				unspentLen := len(unspents[referTxnHash])
				for k, outputIndex := range unspents[referTxnHash] {
					if outputIndex == uint16(txn.UTXOInputs[index].ReferTxOutputIndex) {
						unspents[referTxnHash][k] = unspents[referTxnHash][unspentLen-1]
						unspents[referTxnHash] = unspents[referTxnHash][:unspentLen-1]
						break
					}
				}
			}
		}
	}

	for txhash, value := range unspents {
		key := bytes.NewBuffer(nil)
		key.WriteByte(byte(IX_Unspent))
		txhash.Serialize(key)

		if len(value) == 0 {
			db.BatchDelete(key.Bytes())
		} else {
			unspentArray := ToByteArray(value)
			db.BatchPut(key.Bytes(), unspentArray)
		}
	}

	return nil
}

func (db *ChainStore) RollbackUnspend(b *Block) error {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspents := make(map[Uint256][]uint16)
	for _, txn := range b.Transactions {
		if txn.TxType == tx.RegisterAsset {
			continue
		}
		// remove all utxos created by this transaction
		txnHash := txn.Hash()
		if err := db.BatchDelete(append(unspentPrefix, txnHash.Bytes()...)); err != nil {
			return err
		}
		if !txn.IsCoinBaseTx() {

			for _, input := range txn.UTXOInputs {
				referTxnHash := input.ReferTxID
				referTxnOutIndex := input.ReferTxOutputIndex
				if _, ok := unspents[referTxnHash]; !ok {
					var err error
					unspentValue, _ := db.Get(append(unspentPrefix, referTxnHash.Bytes()...))
					if len(unspentValue) != 0 {
						unspents[referTxnHash], err = GetUint16Array(unspentValue)
						if err != nil {
							return err
						}
					}
				}
				unspents[referTxnHash] = append(unspents[referTxnHash], referTxnOutIndex)
			}
		}
	}

	for txhash, value := range unspents {
		key := bytes.NewBuffer(nil)
		key.WriteByte(byte(IX_Unspent))
		txhash.Serialize(key)

		if len(value) == 0 {
			db.BatchDelete(key.Bytes())
		} else {
			unspentArray := ToByteArray(value)
			db.BatchPut(key.Bytes(), unspentArray)
		}
	}

	return nil
}
