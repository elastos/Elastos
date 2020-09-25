package blockchain

import (
	"bytes"
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
)

// key: DATA_Header || block hash
// value: sysfee(8bytes) || trimmed block
func (s *ChainStore) persistTrimmedBlock(batch database.Batch, b *types.Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_Header))

	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	value := new(bytes.Buffer)
	var sysfee uint64 = 0x0000000000000000
	if err := common.WriteUint64(value, sysfee); err != nil {
		return err
	}
	if err := b.Trim(value); err != nil {
		return err
	}
	batch.Put(key.Bytes(), value.Bytes())
	return nil
}

func (s *ChainStore) rollbackTrimmedBlock(batch database.Batch, b *types.Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_Header))
	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}
	batch.Delete(key.Bytes())
	return nil
}

// key: DATA_BlockHash || height
// value: block hash
func (s *ChainStore) persistBlockHash(batch database.Batch, b *types.Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_BlockHash))
	if err := common.WriteUint32(key, b.Header.GetHeight()); err != nil {
		return err
	}

	value := new(bytes.Buffer)
	hash := b.Hash()
	if err := hash.Serialize(value); err != nil {
		return err
	}
	batch.Put(key.Bytes(), value.Bytes())
	return nil
}

func (s *ChainStore) rollbackBlockHash(batch database.Batch, b *types.Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_BlockHash))
	if err := common.WriteUint32(key, b.Header.GetHeight()); err != nil {
		return err
	}
	batch.Delete(key.Bytes())
	return nil
}

// key: SYS_CurrentBlock
// value: current block hash || height
func (s *ChainStore) persistCurrentBlock(batch database.Batch, b *types.Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(SYS_CurrentBlock))

	value := new(bytes.Buffer)
	blockHash := b.Hash()
	if err := blockHash.Serialize(value); err != nil {
		return err
	}
	if err := common.WriteUint32(value, b.Header.GetHeight()); err != nil {
		return err
	}
	batch.Put(key.Bytes(), value.Bytes())
	return nil
}

func (s *ChainStore) rollbackCurrentBlock(batch database.Batch, b *types.Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(SYS_CurrentBlock))

	value := bytes.NewBuffer(nil)
	previous := b.Header.GetPrevious()
	if err := previous.Serialize(value); err != nil {
		return err
	}
	if err := common.WriteUint32(value, b.Header.GetHeight() - 1); err != nil {
		return err
	}
	batch.Put(key.Bytes(), value.Bytes())
	return nil
}

func (s *ChainStore) persistUnspendUTXOs(batch database.Batch, b *types.Block) error {
	unspendUTXOs := make(map[common.Uint168]map[common.Uint256]map[uint32][]*types.UTXO)
	curHeight := b.Header.GetHeight()

	for _, txn := range b.Transactions {
		if txn.TxType == types.RegisterAsset {
			continue
		}

		for index, output := range txn.Outputs {
			programHash := output.ProgramHash
			assetID := output.AssetID
			value := output.Value

			if _, ok := unspendUTXOs[programHash]; !ok {
				unspendUTXOs[programHash] = make(map[common.Uint256]map[uint32][]*types.UTXO)
			}

			if _, ok := unspendUTXOs[programHash][assetID]; !ok {
				unspendUTXOs[programHash][assetID] = make(map[uint32][]*types.UTXO, 0)
			}

			if _, ok := unspendUTXOs[programHash][assetID][curHeight]; !ok {
				var err error
				unspendUTXOs[programHash][assetID][curHeight], err = s.GetUnspentElementFromProgramHash(programHash, assetID, curHeight)
				if err != nil {
					unspendUTXOs[programHash][assetID][curHeight] = make([]*types.UTXO, 0)
				}

			}

			u := types.UTXO{
				TxId:  txn.Hash(),
				Index: uint32(index),
				Value: value,
			}
			unspendUTXOs[programHash][assetID][curHeight] = append(unspendUTXOs[programHash][assetID][curHeight], &u)
		}

		if !txn.IsCoinBaseTx() {
			for _, input := range txn.Inputs {
				referTxn, height, err := s.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				index := input.Previous.Index
				referTxnOutput := referTxn.Outputs[index]
				programHash := referTxnOutput.ProgramHash
				assetID := referTxnOutput.AssetID

				if _, ok := unspendUTXOs[programHash]; !ok {
					unspendUTXOs[programHash] = make(map[common.Uint256]map[uint32][]*types.UTXO)
				}
				if _, ok := unspendUTXOs[programHash][assetID]; !ok {
					unspendUTXOs[programHash][assetID] = make(map[uint32][]*types.UTXO)
				}

				if _, ok := unspendUTXOs[programHash][assetID][height]; !ok {
					unspendUTXOs[programHash][assetID][height], err = s.GetUnspentElementFromProgramHash(programHash, assetID, height)

					if err != nil {
						return errors.New(fmt.Sprintf("[persist] UTXOs programHash:%v, assetId:%v height:%v has no unspent UTXO.", programHash, assetID, height))
					}
				}

				flag := false
				listnum := len(unspendUTXOs[programHash][assetID][height])
				for i := 0; i < listnum; i++ {
					if unspendUTXOs[programHash][assetID][height][i].TxId.IsEqual(referTxn.Hash()) && unspendUTXOs[programHash][assetID][height][i].Index == uint32(index) {
						unspendUTXOs[programHash][assetID][height][i] = unspendUTXOs[programHash][assetID][height][listnum-1]
						unspendUTXOs[programHash][assetID][height] = unspendUTXOs[programHash][assetID][height][:listnum-1]
						flag = true
						break
					}
				}
				if !flag {
					return errors.New(fmt.Sprintf("[persist] UTXOs NOT find UTXO by txid: %x, index: %d.", referTxn.Hash(), index))
				}
			}
		}
	}

	// batch put the UTXOs
	for programHash, programHash_value := range unspendUTXOs {
		for assetId, unspents := range programHash_value {
			for height, unspent := range unspents {
				err := s.PersistUnspentWithProgramHash(batch, programHash, assetId, height, unspent)
				if err != nil {
					return err
				}
			}

		}
	}

	return nil
}

func (s *ChainStore) rollbackUnspendUTXOs(batch database.Batch, b *types.Block) error {
	unspendUTXOs := make(map[common.Uint168]map[common.Uint256]map[uint32][]*types.UTXO)
	height := b.Header.GetHeight()
	for _, txn := range b.Transactions {
		if txn.TxType == types.RegisterAsset {
			continue
		}
		for index, output := range txn.Outputs {
			programHash := output.ProgramHash
			assetID := output.AssetID
			value := output.Value
			if _, ok := unspendUTXOs[programHash]; !ok {
				unspendUTXOs[programHash] = make(map[common.Uint256]map[uint32][]*types.UTXO)
			}
			if _, ok := unspendUTXOs[programHash][assetID]; !ok {
				unspendUTXOs[programHash][assetID] = make(map[uint32][]*types.UTXO)
			}
			if _, ok := unspendUTXOs[programHash][assetID][height]; !ok {
				var err error
				unspendUTXOs[programHash][assetID][height], err = s.GetUnspentElementFromProgramHash(programHash, assetID, height)
				if err != nil {
					return errors.New(fmt.Sprintf("[persist] UTXOs programHash:%v, assetId:%v has no unspent UTXO.", programHash, assetID))
				}
			}
			u := types.UTXO{
				TxId:  txn.Hash(),
				Index: uint32(index),
				Value: value,
			}
			var position int
			for i, unspend := range unspendUTXOs[programHash][assetID][height] {
				if unspend.TxId == u.TxId && unspend.Index == u.Index {
					position = i
					break
				}
			}
			unspendUTXOs[programHash][assetID][height] = append(unspendUTXOs[programHash][assetID][height][:position], unspendUTXOs[programHash][assetID][height][position+1:]...)
		}

		if !txn.IsCoinBaseTx() {
			for _, input := range txn.Inputs {
				referTxn, hh, err := s.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				index := input.Previous.Index
				referTxnOutput := referTxn.Outputs[index]
				programHash := referTxnOutput.ProgramHash
				assetID := referTxnOutput.AssetID
				if _, ok := unspendUTXOs[programHash]; !ok {
					unspendUTXOs[programHash] = make(map[common.Uint256]map[uint32][]*types.UTXO)
				}
				if _, ok := unspendUTXOs[programHash][assetID]; !ok {
					unspendUTXOs[programHash][assetID] = make(map[uint32][]*types.UTXO)
				}
				if _, ok := unspendUTXOs[programHash][assetID][hh]; !ok {
					unspendUTXOs[programHash][assetID][hh], err = s.GetUnspentElementFromProgramHash(programHash, assetID, hh)
					if err != nil {
						unspendUTXOs[programHash][assetID][hh] = make([]*types.UTXO, 0)
					}
				}
				u := types.UTXO{
					TxId:  referTxn.Hash(),
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
				err := s.PersistUnspentWithProgramHash(batch, programHash, assetId, height, unspent)
				if err != nil {
					return err
				}
			}

		}
	}

	return nil
}

func (s *ChainStore) persistTransactions(batch database.Batch, b *types.Block) error {
	for _, txn := range b.Transactions {
		if err := s.PersistTransaction(batch, txn, b.Header.GetHeight()); err != nil {
			return err
		}
		if txn.TxType == types.RegisterAsset {
			regPayload := txn.Payload.(*types.PayloadRegisterAsset)
			if err := s.PersistAsset(batch, txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}
		if txn.TxType == types.RechargeToSideChain {
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			s.PersistMainchainTx(batch, *hash)
		}
	}
	return nil
}

func (s *ChainStore) PersistTransaction(batch database.Batch, tx *types.Transaction, height uint32) error {
	// generate key with DATA_Transaction prefix
	key := new(bytes.Buffer)
	// add transaction header prefix.
	key.WriteByte(byte(DATA_Transaction))
	// get transaction hash
	hash := tx.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}
	log.Debugf("transaction header + hash: %x", key)

	// generate value
	value := new(bytes.Buffer)
	common.WriteUint32(value, height)
	if err := tx.Serialize(value); err != nil {
		return err
	}
	log.Debugf("transaction tx data: %x", value)

	return batch.Put(key.Bytes(), value.Bytes())
}

func (s *ChainStore) rollbackTransactions(batch database.Batch, b *types.Block) error {
	for _, txn := range b.Transactions {
		if err := s.RollbackTransaction(batch, txn); err != nil {
			return err
		}
		if txn.TxType == types.RegisterAsset {
			if err := s.RollbackAsset(batch, txn.Hash()); err != nil {
				return err
			}
		}
		if txn.TxType == types.RechargeToSideChain {
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			s.RollbackMainchainTx(batch, *hash)
		}
	}

	return nil
}

func (s *ChainStore) RollbackTransaction(batch database.Batch, txn *types.Transaction) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_Transaction))
	hash := txn.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}
	batch.Delete(key.Bytes())
	return nil
}

func (s *ChainStore) RollbackAsset(batch database.Batch, assetId common.Uint256) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(ST_Info))
	assetId.Serialize(key)
	batch.Delete(key.Bytes())
	return nil
}

func (s *ChainStore) RollbackMainchainTx(batch database.Batch, mainchainTxHash common.Uint256) error {
	key := []byte{byte(IX_MainChain_Tx)}
	key = append(key, mainchainTxHash.Bytes()...)

	batch.Delete(key)
	return nil
}

func (s *ChainStore) persistUnspend(batch database.Batch, b *types.Block) error {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspents := make(map[common.Uint256][]uint16)
	for _, txn := range b.Transactions {
		if txn.TxType == types.RegisterAsset {
			continue
		}
		txnHash := txn.Hash()
		for index := range txn.Outputs {
			unspents[txnHash] = append(unspents[txnHash], uint16(index))
		}
		if !txn.IsCoinBaseTx() {
			for index, input := range txn.Inputs {
				referTxnHash := input.Previous.TxID
				if _, ok := unspents[referTxnHash]; !ok {
					unspentValue, err := s.Get(append(unspentPrefix, referTxnHash.Bytes()...))
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
					if outputIndex == uint16(txn.Inputs[index].Previous.Index) {
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
			batch.Delete(key.Bytes())
		} else {
			unspentArray := ToByteArray(value)
			batch.Put(key.Bytes(), unspentArray)
		}
	}

	return nil
}

func (s *ChainStore) rollbackUnspend(batch database.Batch, b *types.Block) error {
	unspentPrefix := []byte{byte(IX_Unspent)}
	unspents := make(map[common.Uint256][]uint16)
	for _, txn := range b.Transactions {
		if txn.TxType == types.RegisterAsset {
			continue
		}
		// remove all utxos created by this transaction
		txnHash := txn.Hash()
		batch.Delete(append(unspentPrefix, txnHash.Bytes()...))
		if !txn.IsCoinBaseTx() {

			for _, input := range txn.Inputs {
				referTxnHash := input.Previous.TxID
				referTxnOutIndex := input.Previous.Index
				if _, ok := unspents[referTxnHash]; !ok {
					var err error
					unspentValue, _ := s.Get(append(unspentPrefix, referTxnHash.Bytes()...))
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
			batch.Delete(key.Bytes())
		} else {
			unspentArray := ToByteArray(value)
			batch.Put(key.Bytes(), unspentArray)
		}
	}

	return nil
}

func GetUint16Array(source []byte) ([]uint16, error) {
	if source == nil {
		return nil, errors.New("[Common] , GetUint16Array err, source = nil")
	}

	if len(source)%2 != 0 {
		return nil, errors.New("[Common] , GetUint16Array err, length of source is odd.")
	}

	dst := make([]uint16, len(source)/2)
	for i := 0; i < len(source)/2; i++ {
		dst[i] = uint16(source[i*2]) + uint16(source[i*2+1])*256
	}

	return dst, nil
}

func ToByteArray(source []uint16) []byte {
	dst := make([]byte, len(source)*2)
	for i := 0; i < len(source); i++ {
		dst[i*2] = byte(source[i] % 256)
		dst[i*2+1] = byte(source[i] / 256)
	}

	return dst
}
