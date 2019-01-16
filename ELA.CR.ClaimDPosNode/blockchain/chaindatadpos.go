package blockchain

import (
	"bytes"
	"errors"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
)

func (c *ChainStore) persistRegisterProducerForMempool(payload *PayloadRegisterProducer, height uint32) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	pk := BytesToHexString(payload.OwnPublicKey)
	c.producerVotes[pk] = &ProducerInfo{
		Payload:   payload,
		RegHeight: height,
		Vote:      Fixed64(0),
	}
	programHash, err := contract.PublicKeyToStandardProgramHash(payload.OwnPublicKey)
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

func (c *ChainStore) rollbackRegisterProducerForMempool(payload *PayloadRegisterProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	_, ok := c.producerVotes[BytesToHexString(payload.OwnPublicKey)]
	if !ok {
		return errors.New("[rollbackRegisterProducer], Not found producer in mempool.")
	}
	delete(c.producerVotes, BytesToHexString(payload.OwnPublicKey))

	programHash, err := contract.PublicKeyToStandardProgramHash(payload.OwnPublicKey)
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

func (c *ChainStore) persistCancelProducerForMempool(payload *PayloadCancelProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	_, ok := c.producerVotes[BytesToHexString(payload.OwnPublicKey)]
	if !ok {
		return errors.New("[persistCancelProducer], Not found producer in mempool.")
	}
	delete(c.producerVotes, BytesToHexString(payload.OwnPublicKey))

	programHash, err := contract.PublicKeyToStandardProgramHash(payload.OwnPublicKey)
	if err != nil {
		return err
	}
	addr, err := programHash.ToAddress()
	if err != nil {
		return err
	}
	delete(c.producerAddress, addr)
	c.canceledProducers[BytesToHexString(payload.OwnPublicKey)] = c.currentBlockHeight
	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) rollbackCancelOrUpdateProducerForMempool() error {
	return c.reloadProducersFromChainForMempool()
}

func (c *ChainStore) reloadProducersFromChainForMempool() error {
	height := c.currentBlockHeight
	for i := config.Parameters.VoteHeight; i <= height; i++ {
		hash, err := c.GetBlockHash(i)
		if err != nil {
			return err
		}
		block, err := c.GetBlock(hash)
		if err != nil {
			return err
		}
		c.persistForMempool(block)
	}
	return nil
}

func (c *ChainStore) persistUpdateProducerForMempool(payload *PayloadUpdateProducer) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	info, ok := c.producerVotes[BytesToHexString(payload.OwnPublicKey)]
	if !ok {
		return errors.New("[persistCancelProducer], Not found producer in mempool.")
	}
	info.Payload = ConvertToRegisterProducerPayload(payload)
	c.dirty[outputpayload.Delegate] = true
	return nil
}

func (c *ChainStore) persistVoteOutputForMempool(output *Output) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	pyaload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
	if !ok {
		return errors.New("[PersistVoteOutput] invalid output payload")
	}

	for _, vote := range pyaload.Contents {
		if vote.VoteType == outputpayload.Delegate {
			for _, candidate := range vote.Candidates {
				if _, ok := c.producerVotes[BytesToHexString(candidate)]; ok {
					c.producerVotes[BytesToHexString(candidate)].Vote += output.Value
				}
			}
			c.dirty[outputpayload.Delegate] = true
		} else {
			// todo persist other vote
		}
	}

	return nil
}

func (c *ChainStore) persistCancelVoteOutputForMempool(output *Output) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	pyaload, ok := output.OutputPayload.(*outputpayload.VoteOutput)
	if !ok {
		return errors.New("[PersistVoteOutput] invalid output payload")
	}

	for _, vote := range pyaload.Contents {
		if vote.VoteType == outputpayload.Delegate {
			for _, candidate := range vote.Candidates {
				if _, ok := c.producerVotes[BytesToHexString(candidate)]; ok {
					c.producerVotes[BytesToHexString(candidate)].Vote -= output.Value
				}
			}
			c.dirty[vote.VoteType] = true
		} else {
			// todo canel other vote
		}
	}

	return nil
}

func (c *ChainStore) clearRegisteredProducerForMempool() {
	// clean from mempool
	c.producerVotes = make(map[string]*ProducerInfo)
	c.producerAddress = make(map[string]string)
	c.dirty = make(map[outputpayload.VoteType]bool)
	c.orderedProducers = make([]*PayloadRegisterProducer, 0)
	c.canceledProducers = make(map[string]uint32)
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

func (c *ChainStore) persistIllegalBlock(illegalBlocks *PayloadIllegalBlock) error {
	if err := c.persistIllegalPayload(func() []string {
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
	}); err != nil {
		return err
	}

	return nil
}

func (c *ChainStore) persistIllegalProposal(payload *PayloadIllegalProposal) error {
	return c.persistIllegalPayload(func() []string {
		return []string{payload.Evidence.Proposal.Sponsor}
	})
}

func (c *ChainStore) persistIllegalVote(payload *PayloadIllegalVote) error {
	return c.persistIllegalPayload(func() []string {
		return []string{payload.Evidence.Vote.Signer}
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

func (c *ChainStore) persistForMempool(b *Block) error {
	for _, txn := range b.Transactions {
		if err := c.persistForVoteInputs(txn); err != nil {
			return err
		}
		switch txn.TxType {
		case RegisterProducer:
			if err := c.persistRegisterProducerForMempool(txn.Payload.(*PayloadRegisterProducer), b.Height); err != nil {
				return err
			}
		case CancelProducer:
			if err := c.persistCancelProducerForMempool(txn.Payload.(*PayloadCancelProducer)); err != nil {
				return err
			}
		case UpdateProducer:
			if err := c.persistUpdateProducerForMempool(txn.Payload.(*PayloadUpdateProducer)); err != nil {
				return err
			}
		case TransferAsset:
			if txn.Version < TxVersion09 {
				break
			}
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					if err := c.persistVoteOutputForMempool(output); err != nil {
						return err
					}
				}
			}
		case IllegalProposalEvidence, IllegalVoteEvidence, IllegalBlockEvidence:
			c.setDirty(outputpayload.Delegate)
		}
	}
	return nil
}

func (c *ChainStore) persistForVoteInputs(tx *Transaction) error {
	if tx.TxType == CoinBase {
		return nil
	}
	for _, input := range tx.Inputs {
		transaction, _, err := c.GetTransaction(input.Previous.TxID)
		if err != nil {
			return err
		}
		if transaction.Version < TxVersion09 {
			continue
		}
		output := transaction.Outputs[input.Previous.Index]
		if output.OutputType == VoteOutput {
			if err = c.persistCancelVoteOutputForMempool(output); err != nil {
				return err
			}
		}
	}
	return nil
}

func (c *ChainStore) rollbackForMempool(b *Block) error {
	for _, txn := range b.Transactions {
		if err := c.rollbackForVoteInputs(txn); err != nil {
			return err
		}
		switch txn.TxType {
		case RegisterProducer:
			regPayload := txn.Payload.(*PayloadRegisterProducer)
			if err := c.rollbackRegisterProducerForMempool(regPayload); err != nil {
				return err
			}
		case CancelProducer, UpdateProducer:
			c.clearRegisteredProducerForMempool()
			if err := c.rollbackCancelOrUpdateProducerForMempool(); err != nil {
				return err
			}
			return nil
		case TransferAsset:
			if txn.Version < TxVersion09 {
				break
			}
			for _, output := range txn.Outputs {
				if output.OutputType == VoteOutput {
					if err := c.persistCancelVoteOutputForMempool(output); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func (c *ChainStore) rollbackForVoteInputs(tx *Transaction) error {
	for _, input := range tx.Inputs {
		transaction, _, err := c.GetTransaction(input.Previous.TxID)
		if err != nil {
			return err
		}
		if transaction.Version < TxVersion09 {
			continue
		}
		output := transaction.Outputs[input.Previous.Index]
		if output.OutputType == VoteOutput {
			if err = c.persistVoteOutputForMempool(output); err != nil {
				return err
			}
		}
	}
	return nil
}
