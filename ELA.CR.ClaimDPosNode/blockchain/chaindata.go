package blockchain

import (
	"bytes"
	"errors"
	"fmt"

	. "github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
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

func (c *ChainStore) persistUnspendUTXOs(b *Block) error {
	unspendUTXOs := make(map[Uint168]map[Uint256]map[uint32][]*UTXO)
	curHeight := b.Header.Height

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
				unspendUTXOs[programHash][assetID] = make(map[uint32][]*UTXO, 0)
			}

			if _, ok := unspendUTXOs[programHash][assetID][curHeight]; !ok {
				var err error
				unspendUTXOs[programHash][assetID][curHeight], err = c.GetUnspentElementFromProgramHash(programHash, assetID, curHeight)
				if err != nil {
					unspendUTXOs[programHash][assetID][curHeight] = make([]*UTXO, 0)
				}

			}

			u := UTXO{
				TxID:  txn.Hash(),
				Index: uint32(index),
				Value: value,
			}
			unspendUTXOs[programHash][assetID][curHeight] = append(unspendUTXOs[programHash][assetID][curHeight], &u)
		}

		if !txn.IsCoinBaseTx() {
			for _, input := range txn.Inputs {
				referTxn, height, err := c.GetTransaction(input.Previous.TxID)
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

				if _, ok := unspendUTXOs[programHash][assetID][height]; !ok {
					unspendUTXOs[programHash][assetID][height], err = c.GetUnspentElementFromProgramHash(programHash, assetID, height)

					if err != nil {
						return errors.New(fmt.Sprintf("[persist] UTXOs programHash:%v, assetID:%v height:%v has no unspent UTXO.", programHash, assetID, height))
					}
				}

				flag := false
				listnum := len(unspendUTXOs[programHash][assetID][height])
				for i := 0; i < listnum; i++ {
					if unspendUTXOs[programHash][assetID][height][i].TxID.IsEqual(referTxn.Hash()) && unspendUTXOs[programHash][assetID][height][i].Index == uint32(index) {
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
	voteOutputs := make([]*Output, 0)
	cancelVoteOutputs := make([]*Output, 0)
	rProducers := make([]*PayloadRegisterProducer, 0)
	cProducers := make([]*PayloadCancelProducer, 0)
	for _, txn := range b.Transactions {
		if err := c.persistTransaction(txn, b.Header.Height); err != nil {
			return err
		}
		if txn.TxType == RegisterAsset {
			regPayload := txn.Payload.(*PayloadRegisterAsset)
			if err := c.PersistAsset(txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}
		if txn.TxType == WithdrawFromSideChain {
			witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
			for _, hash := range witPayload.SideChainTransactionHashes {
				c.PersistSidechainTx(hash)
			}
		}
		if txn.TxType == RegisterProducer {
			rPayload := txn.Payload.(*PayloadRegisterProducer)
			rProducers = append(rProducers, rPayload)
			err := c.persistRegisterProducer(rPayload)
			if err != nil {
				return err
			}
		}
		if txn.TxType == CancelProducer {
			cPayload := txn.Payload.(*PayloadCancelProducer)
			cProducers = append(cProducers, cPayload)
			err := c.persistCancelProducer(cPayload)
			if err != nil {
				return err
			}
		}
		if txn.TxType == UpdateProducer {
			if err := c.persistUpdateProducer(txn.Payload.(*PayloadUpdateProducer)); err != nil {
				return err
			}
		}
		if txn.TxType == TransferAsset && txn.Version >= TxVersion09 {
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					voteOutputs = append(voteOutputs, output)
				}
			}
			for _, input := range txn.Inputs {
				transaction, _, err := c.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				output := transaction.Outputs[input.Previous.Index]
				if output.OutputType == VoteOutput {
					cancelVoteOutputs = append(cancelVoteOutputs, output)
				}
			}
		}
		if txn.TxType == IllegalProposalEvidence {
			if err := c.persistIllegalProposal(txn.Payload.(*PayloadIllegalProposal)); err != nil {
				return err
			}
		}
		if txn.TxType == IllegalVoteEvidence {
			if err := c.persistIllegalVote(txn.Payload.(*PayloadIllegalVote)); err != nil {
				return err
			}
		}
		if txn.TxType == IllegalBlockEvidence {
			if err := c.persistIllegalBlock(txn.Payload.(*PayloadIllegalBlock)); err != nil {
				return err
			}
		}
	}
	if err := c.persistProducers(rProducers, cProducers); err != nil {
		return err
	}
	if err := c.persistVoteOutputs(voteOutputs, cancelVoteOutputs); err != nil {
		return err
	}

	return nil
}

func (c *ChainStore) RollbackTransactions(b *Block) error {
	rollbackedProducer := false
	voteOutputs := make([]*Output, 0)
	cancelVoteOutputs := make([]*Output, 0)
	rProducers := make([]*PayloadRegisterProducer, 0)
	for _, txn := range b.Transactions {
		if err := c.rollbackTransaction(txn); err != nil {
			return err
		}
		if txn.TxType == RegisterAsset {
			if err := c.rollbackAsset(txn.Hash()); err != nil {
				return err
			}
		}
		if txn.TxType == WithdrawFromSideChain {
			witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
			for _, hash := range witPayload.SideChainTransactionHashes {
				if err := c.rollbackSidechainTx(hash); err != nil {
					return err
				}
			}
		}
		if !rollbackedProducer && (txn.TxType == CancelProducer || txn.TxType == UpdateProducer) {
			if err := c.clearRegisteredProducer(); err != nil {
				return err
			}
			if err := c.rollbackCancelOrUpdateProducer(); err != nil {
				return err
			}
			rollbackedProducer = true
		}
		if txn.TxType == RegisterProducer && !rollbackedProducer {
			rPayload := txn.Payload.(*PayloadRegisterProducer)
			rProducers = append(rProducers, rPayload)
			if err := c.rollbackRegisterProducer(rPayload); err != nil {
				return err
			}
		}
		if txn.TxType == TransferAsset && txn.Version >= TxVersion09 && !rollbackedProducer {
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					voteOutputs = append(voteOutputs, output)
				}
			}
			for _, input := range txn.Inputs {
				transaction, _, err := c.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				output := transaction.Outputs[input.Previous.Index]
				if output.OutputType == VoteOutput {
					cancelVoteOutputs = append(cancelVoteOutputs, output)
				}
			}
		}
	}
	if !rollbackedProducer {
		if err := c.rollbackProducers(rProducers); err != nil {
			return err
		}
		if err := c.rollbackVoteOutputs(voteOutputs, cancelVoteOutputs); err != nil {
			return err
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

func (c *ChainStore) PersistConfirm(confirm *DPosProposalVoteSlot) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATAConfirm))
	if err := confirm.Hash.Serialize(key); err != nil {
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
