package blockchain

import (
	"bytes"
	"errors"
	"fmt"

	. "github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// key: DATAHeader || block hash
// value: sysfee(8bytes) || trimmed block
func (c *ChainStore) persistTrimmedBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATAHeader))
	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	value := new(bytes.Buffer)
	var sysFee uint64 = 0x0000000000000000
	if err := WriteUint64(value, sysFee); err != nil {
		return err
	}
	if err := b.Header.Serialize(value); err != nil {
		return err
	}
	if err := WriteUint32(value, uint32(len(b.Transactions))); err != nil {
		return err
	}
	for _, tx := range b.Transactions {
		txHash := tx.Hash()
		if err := txHash.Serialize(value); err != nil {
			return errors.New("Block item transaction hash serialize failed, " + err.Error())
		}
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) RollbackTrimmedBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATAHeader))
	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

// key: DATABlockHash || height
// value: block hash
func (c *ChainStore) persistBlockHash(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATABlockHash))
	if err := WriteUint32(key, b.Header.Height); err != nil {
		return err
	}

	value := new(bytes.Buffer)
	hash := b.Hash()
	if err := hash.Serialize(value); err != nil {
		return err
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) RollbackBlockHash(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATABlockHash))
	if err := WriteUint32(key, b.Header.Height); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

// key: SYSCurrentBlock
// value: current block hash || height
func (c *ChainStore) persistCurrentBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(SYSCurrentBlock))

	value := new(bytes.Buffer)
	hash := b.Hash()
	if err := hash.Serialize(value); err != nil {
		return err
	}
	if err := WriteUint32(value, b.Header.Height); err != nil {
		return err
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) RollbackCurrentBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(SYSCurrentBlock))

	value := new(bytes.Buffer)
	hash := b.Header.Previous
	if err := hash.Serialize(value); err != nil {
		return err
	}
	if err := WriteUint32(value, b.Header.Height-1); err != nil {
		return err
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) persistUTXOs(b *Block) error {
	utxos := make(map[Uint168]map[Uint256]map[uint32][]*UTXO)
	curHeight := b.Header.Height

	for _, txn := range b.Transactions {
		if txn.TxType == RegisterAsset {
			continue
		}

		// Store UTXOs according to the transaction outputs.
		for index, output := range txn.Outputs {
			programHash := output.ProgramHash
			assetID := output.AssetID
			value := output.Value

			if _, ok := utxos[programHash]; !ok {
				utxos[programHash] = make(map[Uint256]map[uint32][]*UTXO)
			}

			if _, ok := utxos[programHash][assetID]; !ok {
				utxos[programHash][assetID] = make(map[uint32][]*UTXO, 0)
			}

			elements, ok := utxos[programHash][assetID][curHeight]
			if !ok {
				elements, _ = c.GetUnspentElementFromProgramHash(programHash, assetID, curHeight)
			}
			elements = append(elements, &UTXO{TxID: txn.Hash(), Index: uint32(index),
				Value: value})

			utxos[programHash][assetID][curHeight] = elements
		}

		// Skip coin base transaction, it has no inputs.
		if txn.IsCoinBaseTx() {
			continue
		}

		// Remove UTXOs according to the transaction inputs.
		for _, input := range txn.Inputs {
			// Find the reference transaction of the input.
			tx, height, err := c.GetTransaction(input.Previous.TxID)
			if err != nil {
				return err
			}
			index := input.Previous.Index
			output := tx.Outputs[index]
			programHash := output.ProgramHash
			assetID := output.AssetID

			if _, ok := utxos[programHash]; !ok {
				utxos[programHash] = make(map[Uint256]map[uint32][]*UTXO)
			}
			if _, ok := utxos[programHash][assetID]; !ok {
				utxos[programHash][assetID] = make(map[uint32][]*UTXO)
			}

			elements, ok := utxos[programHash][assetID][height]
			if !ok {
				elements, _ = c.GetUnspentElementFromProgramHash(programHash, assetID, height)
			}

			// Find the spent UTXO and remove it.
			for i, e := range elements {
				if !e.TxID.IsEqual(tx.Hash()) {
					continue
				}
				if e.Index != uint32(index) {
					continue
				}

				// Here is a little tricky, we replace the matched UTXO with the
				// last UTXO, and then remove the last UTXO.
				elements[i] = elements[len(elements)-1]
				utxos[programHash][assetID][height] = elements[:len(elements)-1]

				// One input has only one match, so we break when found the
				// match.
				break
			}
		}
	}

	// batch put the UTXOs
	for programHash, assetUTXOs := range utxos {
		for assetID, utxos := range assetUTXOs {
			for height, utxo := range utxos {
				err := c.PersistUnspentWithProgramHash(programHash, assetID, height, utxo)
				if err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func (c *ChainStore) RollbackUnspendUTXOs(b *Block) error {
	unspendUTXOs := make(map[Uint168]map[Uint256]map[uint32][]*UTXO)
	height := b.Header.Height
	for _, txn := range b.Transactions {
		if txn.TxType == RegisterAsset {
			continue
		}
		for index, output := range txn.Outputs {
			programHash := output.ProgramHash
			assetID := output.AssetID
			value := output.Value
			if _, ok := unspendUTXOs[programHash]; !ok {
				unspendUTXOs[programHash] = make(map[Uint256]map[uint32][]*UTXO)
			}
			if _, ok := unspendUTXOs[programHash][assetID]; !ok {
				unspendUTXOs[programHash][assetID] = make(map[uint32][]*UTXO)
			}
			if _, ok := unspendUTXOs[programHash][assetID][height]; !ok {
				var err error
				unspendUTXOs[programHash][assetID][height], err = c.GetUnspentElementFromProgramHash(programHash, assetID, height)
				if err != nil {
					return errors.New(fmt.Sprintf("[persist] UTXOs programHash:%v, assetID:%v has no unspent UTXO.", programHash, assetID))
				}
			}
			u := UTXO{
				TxID:  txn.Hash(),
				Index: uint32(index),
				Value: value,
			}
			var position int
			for i, unspend := range unspendUTXOs[programHash][assetID][height] {
				if unspend.TxID == u.TxID && unspend.Index == u.Index {
					position = i
					break
				}
			}
			unspendUTXOs[programHash][assetID][height] = append(unspendUTXOs[programHash][assetID][height][:position], unspendUTXOs[programHash][assetID][height][position+1:]...)
		}

		if !txn.IsCoinBaseTx() {
			for _, input := range txn.Inputs {
				referTxn, hh, err := c.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				index := input.Previous.Index
				referTxnOutput := referTxn.Outputs[index]
				programHash := referTxnOutput.ProgramHash
				assetID := referTxnOutput.AssetID
				if _, ok := unspendUTXOs[programHash]; !ok {
					unspendUTXOs[programHash] = make(map[Uint256]map[uint32][]*UTXO)
				}
				if _, ok := unspendUTXOs[programHash][assetID]; !ok {
					unspendUTXOs[programHash][assetID] = make(map[uint32][]*UTXO)
				}
				if _, ok := unspendUTXOs[programHash][assetID][hh]; !ok {
					unspendUTXOs[programHash][assetID][hh], err = c.GetUnspentElementFromProgramHash(programHash, assetID, hh)
					if err != nil {
						unspendUTXOs[programHash][assetID][hh] = make([]*UTXO, 0)
					}
				}
				u := UTXO{
					TxID:  referTxn.Hash(),
					Index: uint32(index),
					Value: referTxnOutput.Value,
				}
				unspendUTXOs[programHash][assetID][hh] = append(unspendUTXOs[programHash][assetID][hh], &u)
			}
		}
	}

	for programHash, programHashValue := range unspendUTXOs {
		for assetID, unspents := range programHashValue {
			for height, unspent := range unspents {
				err := c.PersistUnspentWithProgramHash(programHash, assetID, height, unspent)
				if err != nil {
					return err
				}
			}

		}
	}

	return nil
}

func (c *ChainStore) PersistTransactions(b *Block) error {
	for _, txn := range b.Transactions {
		if err := c.persistTransaction(txn, b.Header.Height); err != nil {
			return err
		}

		switch txn.TxType {
		case RegisterAsset:
			regPayload := txn.Payload.(*payload.RegisterAsset)
			if err := c.PersistAsset(txn.Hash(), regPayload.Asset); err != nil {
				return err
			}

		case WithdrawFromSideChain:
			witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
			for _, hash := range witPayload.SideChainTransactionHashes {
				c.PersistSidechainTx(hash)
			}
		}
	}

	return nil
}

func (c *ChainStore) RollbackTransactions(b *Block) error {
	for _, txn := range b.Transactions {
		if err := c.rollbackTransaction(txn); err != nil {
			return err
		}
		switch txn.TxType {
		case RegisterAsset:
			if err := c.rollbackAsset(txn.Hash()); err != nil {
				return err
			}
		case WithdrawFromSideChain:
			witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
			for _, hash := range witPayload.SideChainTransactionHashes {
				if err := c.rollbackSidechainTx(hash); err != nil {
					return err
				}
			}
		}
	}
	return nil
}

func (c *ChainStore) rollbackTransaction(txn *Transaction) error {

	key := new(bytes.Buffer)
	key.WriteByte(byte(DATATransaction))
	hash := txn.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

func (c *ChainStore) rollbackAsset(assetID Uint256) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(STInfo))
	if err := assetID.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

func (c *ChainStore) rollbackSidechainTx(sidechainTxHash Uint256) error {
	key := []byte{byte(IXSideChainTx)}
	key = append(key, sidechainTxHash.Bytes()...)

	c.BatchDelete(key)
	return nil
}

func (c *ChainStore) persistUnspend(b *Block) error {
	unspentPrefix := []byte{byte(IXUnspent)}
	unspents := make(map[Uint256][]uint16)
	for _, txn := range b.Transactions {
		if txn.TxType == RegisterAsset {
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
					unspentValue, err := c.Get(append(unspentPrefix, referTxnHash.Bytes()...))
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
		key := new(bytes.Buffer)
		key.WriteByte(byte(IXUnspent))
		txhash.Serialize(key)

		if len(value) == 0 {
			c.BatchDelete(key.Bytes())
		} else {
			unspentArray := ToByteArray(value)
			c.BatchPut(key.Bytes(), unspentArray)
		}
	}

	return nil
}

func (c *ChainStore) RollbackUnspend(b *Block) error {
	unspentPrefix := []byte{byte(IXUnspent)}
	unspents := make(map[Uint256][]uint16)
	for _, txn := range b.Transactions {
		if txn.TxType == RegisterAsset {
			continue
		}
		// remove all utxos created by this transaction
		txnHash := txn.Hash()
		c.BatchDelete(append(unspentPrefix, txnHash.Bytes()...))
		if !txn.IsCoinBaseTx() {

			for _, input := range txn.Inputs {
				referTxnHash := input.Previous.TxID
				referTxnOutIndex := input.Previous.Index
				if _, ok := unspents[referTxnHash]; !ok {
					var err error
					unspentValue, _ := c.Get(append(unspentPrefix, referTxnHash.Bytes()...))
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
		key := new(bytes.Buffer)
		key.WriteByte(byte(IXUnspent))
		txhash.Serialize(key)

		if len(value) == 0 {
			c.BatchDelete(key.Bytes())
		} else {
			unspentArray := ToByteArray(value)
			c.BatchPut(key.Bytes(), unspentArray)
		}
	}

	return nil
}

func (c *ChainStore) PersistConfirm(
	confirm *payload.Confirm) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATAConfirm))
	if err := confirm.Proposal.BlockHash.Serialize(key); err != nil {
		return err
	}

	value := new(bytes.Buffer)
	if err := confirm.Serialize(value); err != nil {
		return err
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) RollbackConfirm(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATAConfirm))
	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
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
