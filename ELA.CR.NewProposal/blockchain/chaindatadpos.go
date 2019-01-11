package blockchain

import (
	"bytes"
	"errors"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
)

func (c *ChainStore) PersistRegisterProducer(payload *PayloadRegisterProducer) error {
	// key: DPOSProducers  value: len,producer1,producer2,producer3
	key := []byte{byte(DPOSProducers)}
	var publicKeys [][]byte
	publicKeys = append(publicKeys, payload.PublicKey)
	pks, err := c.getRegisteredProducers()
	if err == nil {
		publicKeys = append(publicKeys, pks...)
	}

	buf := new(bytes.Buffer)
	if err = WriteVarUint(buf, uint64(len(publicKeys))); err != nil {
		return errors.New("write count failed")
	}
	for _, pk := range publicKeys {
		if err := WriteVarBytes(buf, pk); err != nil {
			return err
		}
	}
	c.BatchPut(key, buf.Bytes())

	// key: DPOSVoteProducer value: height,payload
	key = []byte{byte(DPOSVoteProducer)}
	key = append(key, payload.PublicKey...)
	buf = new(bytes.Buffer)
	height := c.GetHeight()
	if err := WriteUint32(buf, height); err != nil {
		return errors.New("write height failed")
	}
	if err := payload.Serialize(buf, PayloadRegisterProducerVersion); err != nil {
		return errors.New("write payload failed")
	}
	c.BatchPut(key, buf.Bytes())
	return nil
}

func (c *ChainStore) persistRegisterProducerForMempool(payload *PayloadRegisterProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	pk := BytesToHexString(payload.PublicKey)
	c.producerVotes[pk] = &ProducerInfo{
		Payload:   payload,
		RegHeight: c.currentBlockHeight,
		Vote:      Fixed64(0),
	}
	programHash, err := contract.PublicKeyToStandardProgramHash(payload.PublicKey)
	if err != nil {
		return err
	}
	addr, err := programHash.ToAddress()
	if err != nil {
		return err
	}
	c.producerAddress[addr] = pk
	for _, t := range outputpayload.VoteTypes {
		c.dirty[t] = true
	}
	return nil
}

func (c *ChainStore) RollbackRegisterProducer(payload *PayloadRegisterProducer) error {
	// remove from DPOSProducers
	key := []byte{byte(DPOSProducers)}
	var publicKeys [][]byte
	pks, err := c.getRegisteredProducers()
	if err != nil {
		return err
	} else {
		for _, pk := range pks {
			if !bytes.Equal(pk, payload.PublicKey) {
				publicKeys = append(publicKeys, pk)
			}
		}
	}
	buf := new(bytes.Buffer)
	if err = WriteVarUint(buf, uint64(len(publicKeys))); err != nil {
		return errors.New("write count failed")
	}
	for _, pk := range publicKeys {
		if err := WriteVarBytes(buf, pk); err != nil {
			return err
		}
	}
	c.BatchPut(key, buf.Bytes())

	// remove from DPOSVoteProducer
	key = []byte{byte(DPOSVoteProducer)}
	key = append(key, payload.PublicKey...)
	c.BatchDelete(key)

	// remove from voteType
	keyVote := []byte{byte(outputpayload.Delegate)}
	_, err = c.getVoteByPublicKey(outputpayload.Delegate, payload.PublicKey)
	if err == nil {
		c.BatchDelete(append(keyVote, payload.PublicKey...))
	}

	return nil
}

func (c *ChainStore) rollbackRegisterProducerForMempool(payload *PayloadRegisterProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	_, ok := c.producerVotes[BytesToHexString(payload.PublicKey)]
	if !ok {
		return errors.New("[RollbackRegisterProducer], Not found producer in mempool.")
	}
	delete(c.producerVotes, BytesToHexString(payload.PublicKey))

	programHash, err := contract.PublicKeyToStandardProgramHash(payload.PublicKey)
	if err != nil {
		return err
	}
	addr, err := programHash.ToAddress()
	if err != nil {
		return err
	}
	delete(c.producerAddress, addr)
	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) PersistCancelProducer(payload *PayloadCancelProducer) error {
	// remove from DPOSProducers
	key := []byte{byte(DPOSProducers)}
	var publicKeys [][]byte
	pks, err := c.getRegisteredProducers()
	if err != nil {
		return err
	} else {
		for _, pk := range pks {
			if !bytes.Equal(pk, payload.PublicKey) {
				publicKeys = append(publicKeys, pk)
			}
		}
	}
	buf := new(bytes.Buffer)
	if err = WriteVarUint(buf, uint64(len(publicKeys))); err != nil {
		return errors.New("write count failed")
	}
	for _, pk := range publicKeys {
		if err := WriteVarBytes(buf, pk); err != nil {
			return err
		}
	}
	c.BatchPut(key, buf.Bytes())

	// remove from DPOSVoteProducer
	key = []byte{byte(DPOSVoteProducer)}
	key = append(key, payload.PublicKey...)
	c.BatchDelete(key)

	// add to cancel producer database
	key = []byte{byte(DPOSCancelProducer)}
	key = append(key, payload.PublicKey...)
	buf = new(bytes.Buffer)
	err = WriteUint32(buf, c.GetHeight())
	if err != nil {
		return errors.New("write cancel producer height failed")
	}
	c.BatchPut(key, buf.Bytes())

	// remove from voteType
	keyVote := []byte{byte(outputpayload.Delegate)}
	_, err = c.getVoteByPublicKey(outputpayload.Delegate, payload.PublicKey)
	if err == nil {
		c.BatchDelete(append(keyVote, payload.PublicKey...))
	}
	return nil
}

func (c *ChainStore) persistCancelProducerForMempool(payload *PayloadCancelProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	_, ok := c.producerVotes[BytesToHexString(payload.PublicKey)]
	if !ok {
		return errors.New("[PersistCancelProducer], Not found producer in mempool.")
	}
	delete(c.producerVotes, BytesToHexString(payload.PublicKey))

	programHash, err := contract.PublicKeyToStandardProgramHash(payload.PublicKey)
	if err != nil {
		return err
	}
	addr, err := programHash.ToAddress()
	if err != nil {
		return err
	}
	delete(c.producerAddress, addr)
	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) RollbackCancelOrUpdateProducer() error {
	height := c.GetHeight()
	for i := uint32(0); i <= height; i++ {
		hash, err := c.GetBlockHash(height)
		if err != nil {
			return err
		}
		block, err := c.GetBlock(hash)
		if err != nil {
			return err
		}

		voteOutputs := make([]*Output, 0)
		cancelVoteOutputs := make([]*Output, 0)
		for _, tx := range block.Transactions {
			if tx.TxType == RegisterProducer {
				if err = c.PersistRegisterProducer(tx.Payload.(*PayloadRegisterProducer)); err != nil {
					return err
				}
			}
			if tx.TxType == UpdateProducer {
				if err = c.PersistUpdateProducer(tx.Payload.(*PayloadUpdateProducer)); err != nil {
					return err
				}
			}
			if tx.TxType == TransferAsset && tx.Version >= TxVersion09 {
				for _, output := range tx.Outputs {
					if output.OutputType == VoteOutput {
						voteOutputs = append(voteOutputs, output)
					}
				}
				for _, input := range tx.Inputs {
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
		if err := c.persistVoteOutputs(voteOutputs, cancelVoteOutputs); err != nil {
			return err
		}
	}
	return nil
}

func (c *ChainStore) rollbackCancelOrUpdateProducerForMempool() error {
	height := c.GetHeight()
	for i := uint32(0); i <= height; i++ {
		hash, err := c.GetBlockHash(height)
		if err != nil {
			return err
		}
		block, err := c.GetBlock(hash)
		if err != nil {
			return err
		}

		for _, tx := range block.Transactions {
			if tx.TxType == RegisterProducer {
				if err = c.persistRegisterProducerForMempool(tx.Payload.(*PayloadRegisterProducer)); err != nil {
					return err
				}
			}
			if tx.TxType == UpdateProducer {
				if err = c.persistUpdateProducerForMempool(tx.Payload.(*PayloadUpdateProducer)); err != nil {
					return err
				}
			}
			if tx.TxType == TransferAsset && tx.Version >= TxVersion09 {
				for _, output := range tx.Outputs {
					if output.OutputType == VoteOutput {
						if err = c.persistVoteOutputForMempool(output); err != nil {
							return err
						}
					}
				}
				for _, input := range tx.Inputs {
					transaction, _, err := c.GetTransaction(input.Previous.TxID)
					if err != nil {
						return err
					}
					output := transaction.Outputs[input.Previous.Index]
					if output.OutputType == VoteOutput {
						if err = c.persistCancelVoteOutputForMempool(output); err != nil {
							return err
						}
					}
				}
			}
		}
	}
	return nil
}

func (c *ChainStore) PersistUpdateProducer(payload *PayloadUpdateProducer) error {
	height, _, err := c.getProducerInfo(payload.PublicKey)
	if err != nil {
		return nil
	}

	// update producer in database
	key := []byte{byte(DPOSVoteProducer)}
	key = append(key, payload.PublicKey...)

	buf := new(bytes.Buffer)
	if err := WriteUint32(buf, height); err != nil {
		return errors.New("write height failed")
	}
	if err := WriteVarBytes(buf, payload.Data(PayloadRegisterProducerVersion)); err != nil {
		return errors.New("write payload failed")
	}
	c.BatchPut(key, buf.Bytes())
	return nil
}

func (c *ChainStore) persistUpdateProducerForMempool(payload *PayloadUpdateProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	info, ok := c.producerVotes[BytesToHexString(payload.PublicKey)]
	if !ok {
		return errors.New("[PersistCancelProducer], Not found producer in mempool.")
	}
	info.Payload = ConvertToRegisterProducerPayload(payload)
	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) persistVoteOutputs(voteOutputs []*Output, cancelVoteOutputs []*Output) error {
	voteProducerMap := make(map[string]Fixed64)
	for _, output := range voteOutputs {
		payload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
		if !ok {
			continue
		}

		for _, vote := range payload.Contents {
			if vote.VoteType == outputpayload.Delegate {
				for _, candidate := range vote.Candidates {
					if value, ok := voteProducerMap[BytesToHexString(candidate)]; ok {
						voteProducerMap[BytesToHexString(candidate)] = value + output.Value
					} else {
						voteProducerMap[BytesToHexString(candidate)] = output.Value
					}
				}
			} else {
				// todo persist other vote
			}
		}
	}

	for _, output := range cancelVoteOutputs {
		payload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
		if !ok {
			continue
		}

		for _, vote := range payload.Contents {
			if vote.VoteType == outputpayload.Delegate {
				for _, candidate := range vote.Candidates {
					if value, ok := voteProducerMap[BytesToHexString(candidate)]; ok {
						voteProducerMap[BytesToHexString(candidate)] = value - output.Value
					} else {
						voteProducerMap[BytesToHexString(candidate)] = -output.Value
					}
				}
			} else {
				// todo persist other vote
			}
		}
	}

	for k, v := range voteProducerMap {
		stake, err := v.Bytes()
		if err != nil {
			return err
		}
		// add vote to database
		key := []byte{byte(outputpayload.Delegate)}
		candidate, _ := HexStringToBytes(k)
		k := append(key, candidate...)
		oldStake, err := c.getVoteByPublicKey(outputpayload.Delegate, candidate)
		if err != nil {
			c.Put(k, stake)
		} else {
			votes := v + oldStake
			votesBytes, err := votes.Bytes()
			if err != nil {
				return err
			}
			c.Put(k, votesBytes)
		}
	}

	return nil
}

func (c *ChainStore) persistVoteOutputForMempool(output *Output) error {
	pyaload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
	if !ok {
		return errors.New("[PersistVoteOutput] invalid output payload")
	}

	for _, vote := range pyaload.Contents {
		if vote.VoteType == outputpayload.Delegate {
			for _, candidate := range vote.Candidates {
				c.mu.Lock()
				c.producerVotes[BytesToHexString(candidate)].Vote += output.Value
				c.dirty[vote.VoteType] = true
				c.mu.Unlock()
			}
		} else {
			// todo persist other vote
		}
	}

	return nil
}

func (c *ChainStore) persistCancelVoteOutputForMempool(output *Output) error {
	pyaload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
	if !ok {
		return errors.New("[PersistVoteOutput] invalid output payload")
	}

	for _, vote := range pyaload.Contents {
		if vote.VoteType == outputpayload.Delegate {
			for _, candidate := range vote.Candidates {
				c.mu.Lock()
				c.producerVotes[BytesToHexString(candidate)].Vote -= output.Value
				c.dirty[vote.VoteType] = true
				c.mu.Unlock()
			}
		} else {
			// todo canel other vote
		}
	}

	return nil
}

func (c *ChainStore) removeCanceledProducer(publicKey []byte) {
	key := []byte{byte(DPOSCancelProducer)}
	key = append(key, publicKey...)
	c.BatchDelete(key)
	return
}

func (c *ChainStore) clearRegisteredProducer() {
	// clean from database
	publicKeys, err := c.getRegisteredProducers()
	if err != nil {
		return
	}
	key := []byte{byte(DPOSProducers)}
	c.BatchDelete(key)

	for _, pk := range publicKeys {
		key = []byte{byte(DPOSVoteProducer)}
		key = append(key, pk...)
		c.BatchDelete(key)
	}

	// clean from mempool
	c.producerVotes = make(map[string]*ProducerInfo)
	c.producerAddress = make(map[string]string)
	c.dirty = make(map[outputpayload.VoteType]bool)
	c.orderedProducers = make([]*PayloadRegisterProducer, 0)
}

func (c *ChainStore) getRegisteredProducers() ([][]byte, error) {
	key := []byte{byte(DPOSProducers)}
	data, err := c.Get(key)
	if err != nil {
		return nil, err
	}
	r := bytes.NewReader(data)
	count, err := ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}
	var result [][]byte
	for i := uint64(0); i < count; i++ {
		pk, err := ReadVarBytes(r, crypto.COMPRESSEDLEN, "public key")
		if err != nil {
			return nil, err
		}
		result = append(result, pk)
	}

	return result, nil
}

func (c *ChainStore) getProducerInfo(publicKey []byte) (uint32, Payload, error) {
	key := []byte{byte(DPOSVoteProducer)}
	key = append(key, publicKey...)
	data, err := c.Get(key)
	if err != nil {
		return 0, nil, err
	}
	r := bytes.NewReader(data)
	height, err := ReadUint32(r)
	if err != nil {
		return 0, nil, err
	}
	var payload PayloadRegisterProducer
	if err := payload.Deserialize(r, PayloadRegisterProducerVersion); err != nil {
		return 0, nil, err
	}

	return height, &payload, nil
}

func (c *ChainStore) getVoteByPublicKey(voteType outputpayload.VoteType, publicKey []byte) (Fixed64, error) {
	key := []byte{byte(voteType)}
	key = append(key, publicKey...)

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

func (c *ChainStore) SaveIllegalBlock(illegalBlocks *PayloadIllegalBlock) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DPOSIllegalProducer))

	producers := c.getIllegalProducers()

	signers := make(map[string]interface{})
	for _, v := range illegalBlocks.Evidence.Signers {
		signers[BytesToHexString(v)] = nil
	}

	for _, v := range illegalBlocks.CompareEvidence.Signers {
		compareSigner := BytesToHexString(v)
		if _, ok := signers[compareSigner]; ok {
			producers[compareSigner] = struct{}{}
		}
	}

	value := new(bytes.Buffer)
	if err := WriteUint64(value, uint64(len(producers))); err != nil {
		return err
	}

	for k := range producers {
		if err := WriteVarString(value, k); err != nil {
			return err
		}
	}

	if err := c.Put(key.Bytes(), value.Bytes()); err != nil {
		return err
	}
	c.dirty[outputpayload.Delegate] = true

	return nil
}

func (c *ChainStore) PersistIllegalBlock(illegalBlocks *PayloadIllegalBlock) error {
	return c.persistIllegalPayload(func() []string {
		signers := make(map[string]interface{})
		for _, v := range illegalBlocks.Evidence.Signers {
			signers[BytesToHexString(v)] = nil
		}

		result := make([]string, 0)
		for _, v := range illegalBlocks.CompareEvidence.Signers {
			compareSigner := BytesToHexString(v)
			if _, ok := signers[compareSigner]; ok {
				result = append(result, compareSigner)
			}
		}

		return result
	})
}

func (c *ChainStore) PersistSidechainIllegalEvidence(payload *PayloadSidechainIllegalData) error {
	return c.persistIllegalPayload(func() []string {
		return []string{payload.IllegalSigner}
	})
}

func (c *ChainStore) PersistIllegalProposal(payload *PayloadIllegalProposal) error {
	return c.persistIllegalPayload(func() []string {
		return []string{BytesToHexString(payload.Evidence.Proposal.Sponsor)}
	})
}

func (c *ChainStore) PersistIllegalVote(payload *PayloadIllegalVote) error {
	return c.persistIllegalPayload(func() []string {
		return []string{BytesToHexString(payload.Evidence.Vote.Signer)}
	})
}

func (c *ChainStore) setDirty(voteType outputpayload.VoteType) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) persistIllegalPayload(getIllegalProducersFun func() []string) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DPOSIllegalProducer))

	producers := c.getIllegalProducers()

	newProducers := getIllegalProducersFun()
	for _, v := range newProducers {
		producers[v] = struct{}{}
	}

	value := new(bytes.Buffer)
	if err := WriteUint64(value, uint64(len(producers))); err != nil {
		return err
	}

	for k := range producers {
		if err := WriteVarString(value, k); err != nil {
			return err
		}
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) getIllegalProducers() map[string]struct{} {
	result := make(map[string]struct{})
	key := []byte{byte(DPOSIllegalProducer)}
	data, err := c.Get(key)
	if err != nil {
		return result
	}
	r := bytes.NewReader(data)
	count, err := ReadUint64(r)
	if err != nil {
		return result
	}

	for i := uint64(0); i < count; i++ {
		p, err := ReadVarString(r)
		if err != nil {
			return result
		}
		result[p] = struct{}{}
	}

	return result
}

func (c *ChainStore) getCancelProducerHeight(publicKey []byte) (uint32, error) {
	key := []byte{byte(DPOSCancelProducer)}
	key = append(key, publicKey...)

	data, err := c.Get(key)
	if err != nil {
		return 0, err
	}
	r := bytes.NewReader(data)
	height, err := ReadUint32(r)
	if err != nil {
		return 0, err
	}

	return height, nil
}

func (c *ChainStore) rollbackForMempool(b *Block) error {
	for _, txn := range b.Transactions {
		if txn.TxType == RegisterProducer {
			regPayload := txn.Payload.(*PayloadRegisterProducer)
			if err := c.rollbackRegisterProducerForMempool(regPayload); err != nil {
				return err
			}
		}
		if txn.TxType == CancelProducer || txn.TxType == UpdateProducer {
			if err := c.rollbackCancelOrUpdateProducerForMempool(); err != nil {
				return err
			}
		}
		if txn.TxType == TransferAsset && txn.Version >= TxVersion09 {
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					if err := c.persistCancelVoteOutputForMempool(output); err != nil {
						return err
					}
				}
			}
			for _, input := range txn.Inputs {
				transaction, _, err := c.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				output := transaction.Outputs[input.Previous.Index]
				if output.OutputType == VoteOutput {
					if err = c.persistVoteOutputForMempool(output); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func (c *ChainStore) persistForMempool(b *Block) error {
	for _, txn := range b.Transactions {
		if txn.TxType == RegisterProducer {
			err := c.persistRegisterProducerForMempool(txn.Payload.(*PayloadRegisterProducer))
			if err != nil {
				return err
			}
		}
		if txn.TxType == CancelProducer {
			err := c.persistCancelProducerForMempool(txn.Payload.(*PayloadCancelProducer))
			if err != nil {
				return err
			}
		}
		if txn.TxType == UpdateProducer {
			if err := c.persistUpdateProducerForMempool(txn.Payload.(*PayloadUpdateProducer)); err != nil {
				return err
			}
		}
		if txn.TxType == TransferAsset && txn.Version >= TxVersion09 {
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					if err := c.persistVoteOutputForMempool(output); err != nil {
						return err
					}
				}
			}
			for _, input := range txn.Inputs {
				transaction, _, err := c.GetTransaction(input.Previous.TxID)
				if err != nil {
					return err
				}
				output := transaction.Outputs[input.Previous.Index]
				if output.OutputType == VoteOutput {
					if err = c.persistCancelVoteOutputForMempool(output); err != nil {
						return err
					}
				}
			}
		}
		if txn.TxType == IllegalProposalEvidence || txn.TxType == IllegalVoteEvidence || txn.TxType == IllegalBlockEvidence {
			c.setDirty(outputpayload.Delegate)
		}
	}
	return nil

}
