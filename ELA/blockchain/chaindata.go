package blockchain

import (
	"bytes"
	"errors"
	"fmt"

	. "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/core/outputpayload"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

// key: DATA_Header || block hash
// value: sysfee(8bytes) || trimmed block
func (c *ChainStore) PersistTrimmedBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_Header))
	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	value := new(bytes.Buffer)
	var sysFee uint64 = 0x0000000000000000
	if err := WriteUint64(value, sysFee); err != nil {
		return err
	}
	if err := b.Trim(value); err != nil {
		return err
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) RollbackTrimmedBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_Header))
	hash := b.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

// key: DATA_BlockHash || height
// value: block hash
func (c *ChainStore) PersistBlockHash(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_BlockHash))
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
	key.WriteByte(byte(DATA_BlockHash))
	if err := WriteUint32(key, b.Header.Height); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

// key: SYS_CurrentBlock
// value: current block hash || height
func (c *ChainStore) PersistCurrentBlock(b *Block) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(SYS_CurrentBlock))

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
	key.WriteByte(byte(SYS_CurrentBlock))

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

func (c *ChainStore) PersistUnspendUTXOs(b *Block) error {
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
				TxId:  txn.Hash(),
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
				err := c.PersistUnspentWithProgramHash(programHash, assetId, height, unspent)
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
					return errors.New(fmt.Sprintf("[persist] UTXOs programHash:%v, assetId:%v has no unspent UTXO.", programHash, assetID))
				}
			}
			u := UTXO{
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
				err := c.PersistUnspentWithProgramHash(programHash, assetId, height, unspent)
				if err != nil {
					return err
				}
			}

		}
	}

	return nil
}

func (c *ChainStore) PersistRegisterProducer(payload *PayloadRegisterProducer) error {
	key := []byte{byte(VOTE_RegisterProducer)}
	hBuf := new(bytes.Buffer)
	height := c.GetHeight()
	WriteUint32(hBuf, height)
	producerBytes, err := c.getRegisteredProducers()
	if err != nil {
		count := new(bytes.Buffer)
		WriteUint64(count, uint64(1))
		c.BatchPut(key, append(count.Bytes(), append(hBuf.Bytes(), payload.Data(PayloadRegisterProducerVersion)...)...))
		return c.recordProducer(payload, height)
	}
	r := bytes.NewReader(producerBytes)
	length, err := ReadUint64(r)
	if err != nil {
		return err
	}

	for i := uint64(0); i < length; i++ {
		_, err := ReadUint32(r)
		if err != nil {
			return err
		}
		var p PayloadRegisterProducer
		err = p.Deserialize(r, PayloadRegisterProducerVersion)
		if err != nil {
			return err
		}
		if p.NickName == payload.NickName {
			return errors.New("duplicated nickname")
		}
		if p.PublicKey == payload.PublicKey {
			return errors.New("duplicated public key")
		}
	}

	// PUT VALUE: length(uint64),oldProducers(height+payload),newProducer
	value := new(bytes.Buffer)
	WriteUint64(value, length+uint64(1))
	c.BatchPut(key, append(append(value.Bytes(), producerBytes[8:]...),
		append(hBuf.Bytes(), payload.Data(PayloadRegisterProducerVersion)...)...))

	return c.recordProducer(payload, height)
}

func (c *ChainStore) recordProducer(payload *PayloadRegisterProducer, regHeight uint32) error {
	programHash, err := PublicKeyToProgramHash(payload.PublicKey)
	if err != nil {
		return errors.New("[recordProducer]" + err.Error())
	}

	c.mu.Lock()
	defer c.mu.Unlock()
	c.producerVotes[*programHash] = &ProducerInfo{
		Payload:   payload,
		RegHeight: regHeight,
		Vote:      make(map[outputpayload.VoteType]Fixed64, 0),
	}
	return nil
}

func (c *ChainStore) PersistCancelProducer(payload *PayloadCancelProducer) error {
	key := []byte{byte(VOTE_RegisterProducer)}
	producerBytes, err := c.getRegisteredProducers()
	if err != nil {
		return err
	}
	r := bytes.NewReader(producerBytes)
	length, err := ReadUint64(r)
	if err != nil {
		return err
	}

	var newProducerBytes []byte
	var count uint64
	for i := uint64(0); i < length; i++ {
		h, err := ReadUint32(r)
		if err != nil {
			return err
		}
		var p PayloadRegisterProducer
		err = p.Deserialize(r, PayloadRegisterProducerVersion)
		if err != nil {
			return err
		}
		if p.PublicKey != payload.PublicKey {
			buf := new(bytes.Buffer)
			WriteUint32(buf, h)
			p.Serialize(buf, PayloadRegisterProducerVersion)
			newProducerBytes = append(newProducerBytes, buf.Bytes()...)
			count++
		}
	}

	value := new(bytes.Buffer)
	WriteUint64(value, count)
	newProducerBytes = append(value.Bytes(), newProducerBytes...)

	c.BatchPut(key, newProducerBytes)

	programHash, err := PublicKeyToProgramHash(payload.PublicKey)
	if err != nil {
		return errors.New("[PersistCancelProducer]" + err.Error())
	}

	// remove from database
	for _, voteType := range outputpayload.VoteTypes {
		keyVote := []byte{byte(voteType)}

		_, err = c.getProducerVote(voteType, *programHash)
		if err == nil {
			c.BatchDelete(append(keyVote, programHash.Bytes()...))
		}
	}

	// remove from mempool
	c.mu.Lock()
	defer c.mu.Unlock()
	_, ok := c.producerVotes[*programHash]
	if !ok {
		return errors.New("[PersistCancelProducer], Not found producer in mempool.")
	}
	delete(c.producerVotes, *programHash)
	return nil
}

func (c *ChainStore) PersistUpdateProducer(payload *PayloadUpdateProducer) error {
	// update producer in database
	key := []byte{byte(VOTE_RegisterProducer)}
	producerBytes, err := c.getRegisteredProducers()
	if err != nil {
		return err
	}
	r := bytes.NewReader(producerBytes)
	length, err := ReadUint64(r)
	if err != nil {
		return err
	}

	var newProducerBytes []byte
	for i := uint64(0); i < length; i++ {
		h, err := ReadUint32(r)
		if err != nil {
			return err
		}
		var p PayloadRegisterProducer
		err = p.Deserialize(r, PayloadRegisterProducerVersion)
		if err != nil {
			return err
		}
		var pld Payload
		if p.PublicKey == payload.PublicKey {
			pld = payload
		} else {
			pld = &p
		}
		buf := new(bytes.Buffer)
		WriteUint32(buf, h)
		pld.Serialize(buf, PayloadRegisterProducerVersion)
		newProducerBytes = append(newProducerBytes, buf.Bytes()...)
	}

	value := new(bytes.Buffer)
	WriteUint64(value, length)
	newProducerBytes = append(value.Bytes(), newProducerBytes...)

	c.BatchPut(key, newProducerBytes)

	programHash, err := PublicKeyToProgramHash(payload.PublicKey)
	if err != nil {
		return errors.New("[PersistCancelProducer]" + err.Error())
	}

	// update producer in mempool
	c.mu.Lock()
	defer c.mu.Unlock()
	info, ok := c.producerVotes[*programHash]
	if !ok {
		return errors.New("[PersistCancelProducer], Not found producer in mempool.")
	}
	info.Payload = payload.PayloadRegisterProducer
	return nil
}

func (c *ChainStore) PersistVoteOutput(output *Output) error {
	stake, err := output.Value.Bytes()
	if err != nil {
		return err
	}

	pyaload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
	if !ok {
		return errors.New("[PersistVoteOutput] invalid output payload")
	}

	for _, vote := range pyaload.Contents {
		for _, hash := range vote.Candidates {
			// add vote to database
			key := []byte{byte(vote.VoteType)}
			k := append(key, hash.Bytes()...)
			oldStake, err := c.getProducerVote(vote.VoteType, hash)
			if err != nil {
				c.BatchPut(k, stake)
			} else {
				votes := output.Value + oldStake
				votesBytes, err := votes.Bytes()
				if err != nil {
					return err
				}
				c.BatchPut(k, votesBytes)
			}

			// add vote to mempool
			c.mu.Lock()
			if p, ok := c.producerVotes[hash]; ok {
				if v, ok := p.Vote[vote.VoteType]; ok {
					c.producerVotes[hash].Vote[vote.VoteType] = v + output.Value
				} else {
					c.producerVotes[hash].Vote[vote.VoteType] = output.Value
				}
			}
			c.mu.Unlock()
		}
	}

	return nil
}

func (c *ChainStore) PersistCancelVoteOutput(output *Output) error {
	pyaload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
	if !ok {
		return errors.New("[PersistVoteOutput] invalid output payload")
	}

	for _, vote := range pyaload.Contents {
		for _, hash := range vote.Candidates {
			// subtract vote to database
			key := []byte{byte(vote.VoteType)}
			k := append(key, hash.Bytes()...)
			oldStake, err := c.getProducerVote(vote.VoteType, hash)
			if err != nil {
				return nil
			} else {
				votes := oldStake - output.Value
				votesBytes, err := votes.Bytes()
				if err != nil {
					return err
				}
				c.BatchPut(k, votesBytes)
			}

			// subtract vote to mempool
			c.mu.Lock()
			if p, ok := c.producerVotes[hash]; ok {
				if v, ok := p.Vote[vote.VoteType]; ok {
					c.producerVotes[hash].Vote[vote.VoteType] = v - output.Value
				}
			}
			c.mu.Unlock()
		}
	}

	return nil
}

func (c *ChainStore) getRegisteredProducers() ([]byte, error) {
	key := []byte{byte(VOTE_RegisterProducer)}
	data, err := c.Get(key)
	if err != nil {
		return nil, err
	}

	return data, nil
}

func (c *ChainStore) getProducerVote(voteType outputpayload.VoteType, programHash Uint168) (Fixed64, error) {
	key := []byte{byte(voteType)}
	key = append(key, programHash.Bytes()...)

	// PUT VALUE
	data, err := c.Get(key)
	if err != nil {
		return Fixed64(0), err
	}

	value, err := Fixed64FromBytes(data)
	if err != nil {
		return Fixed64(0), err
	}

	return *value, nil
}

func (c *ChainStore) PersistTransactions(b *Block) error {
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(txn, b.Header.Height); err != nil {
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
			err := c.PersistRegisterProducer(txn.Payload.(*PayloadRegisterProducer))
			if err != nil {
				return err
			}
		}
		if txn.TxType == CancelProducer {
			err := c.PersistCancelProducer(txn.Payload.(*PayloadCancelProducer))
			if err != nil {
				return err
			}
		}
		if txn.TxType == UpdateProducer {
			err := c.PersistUpdateProducer(txn.Payload.(*PayloadUpdateProducer))
			if err != nil {
				return err
			}
		}
		if txn.TxType == TransferAsset {
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					c.PersistVoteOutput(output)
				}
			}
			for _, input := range txn.Inputs {
				transaction, _, err := DefaultLedger.Store.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				output := transaction.Outputs[input.Previous.Index]
				if output.OutputType == VoteOutput {
					c.PersistCancelVoteOutput(output)
				}
			}
		}
	}
	return nil
}

func (c *ChainStore) RollbackTransactions(b *Block) error {
	for _, txn := range b.Transactions {
		if err := c.RollbackTransaction(txn); err != nil {
			return err
		}
		if txn.TxType == RegisterAsset {
			if err := c.RollbackAsset(txn.Hash()); err != nil {
				return err
			}
		}
		if txn.TxType == WithdrawFromSideChain {
			witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
			for _, hash := range witPayload.SideChainTransactionHashes {
				if err := c.RollbackSidechainTx(hash); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func (c *ChainStore) RollbackTransaction(txn *Transaction) error {

	key := new(bytes.Buffer)
	key.WriteByte(byte(DATA_Transaction))
	hash := txn.Hash()
	if err := hash.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

func (c *ChainStore) RollbackAsset(assetId Uint256) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(ST_Info))
	if err := assetId.Serialize(key); err != nil {
		return err
	}

	c.BatchDelete(key.Bytes())
	return nil
}

func (c *ChainStore) RollbackSidechainTx(sidechainTxHash Uint256) error {
	key := []byte{byte(IX_SideChain_Tx)}
	key = append(key, sidechainTxHash.Bytes()...)

	c.BatchDelete(key)
	return nil
}

func (c *ChainStore) PersistUnspend(b *Block) error {
	unspentPrefix := []byte{byte(IX_Unspent)}
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
		key.WriteByte(byte(IX_Unspent))
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
	unspentPrefix := []byte{byte(IX_Unspent)}
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
		key.WriteByte(byte(IX_Unspent))
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

func PublicKeyToProgramHash(publicKey string) (*Uint168, error) {
	pkBytes, err := HexStringToBytes(publicKey)
	if err != nil {
		return nil, errors.New("[getProgramHash] public key to bytes")
	}
	programHash, err := crypto.PublicKeyToStandardProgramHash(pkBytes)
	if err != nil {
		return nil, errors.New("[getProgramHash] public key to program hash failed")
	}

	return programHash, nil
}
