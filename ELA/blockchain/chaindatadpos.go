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

	return c.recordProducer(payload, height)
}

func (c *ChainStore) RollbackRegisterProducer(payload *PayloadRegisterProducer) error {
	return c.PersistCancelProducer(&PayloadCancelProducer{PublicKey: payload.PublicKey})
}

func (c *ChainStore) recordProducer(payload *PayloadRegisterProducer, regHeight uint32) error {
	c.mu.Lock()
	defer c.mu.Unlock()
	pk := BytesToHexString(payload.PublicKey)
	c.producerVotes[pk] = &ProducerInfo{
		Payload:   payload,
		RegHeight: regHeight,
		Vote:      make(map[outputpayload.VoteType]Fixed64, 0),
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
	err = WriteUint32(buf, DefaultLedger.Blockchain.GetBestHeight())
	if err != nil {
		return errors.New("write cancel producer height failed")
	}
	c.BatchPut(key, buf.Bytes())

	// remove from voteType
	for _, voteType := range outputpayload.VoteTypes {
		keyVote := []byte{byte(voteType)}
		_, err = c.getProducerVote(voteType, payload.PublicKey)
		if err == nil {
			c.BatchDelete(append(keyVote, payload.PublicKey...))
		}
	}

	// remove from mempool
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
	for _, t := range outputpayload.VoteTypes {
		c.dirty[t] = true
	}
	return nil
}

func (c *ChainStore) RollbackCancelOrUpdateProducer() error {
	height := DefaultLedger.Blockchain.GetBestHeight()
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
						if err = c.PersistVoteOutput(output); err != nil {
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
						if err = c.PersistCancelVoteOutput(output); err != nil {
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

	// update producer in mempool
	c.mu.Lock()
	defer c.mu.Unlock()
	info, ok := c.producerVotes[BytesToHexString(payload.PublicKey)]
	if !ok {
		return errors.New("[PersistCancelProducer], Not found producer in mempool.")
	}
	info.Payload = ConvertToRegisterProducerPayload(payload)
	for _, t := range outputpayload.VoteTypes {
		c.dirty[t] = true
	}
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
		for _, candidate := range vote.Candidates {
			// add vote to database
			key := []byte{byte(vote.VoteType)}
			k := append(key, candidate...)
			oldStake, err := c.getProducerVote(vote.VoteType, candidate)
			if err != nil {
				c.Put(k, stake)
			} else {
				votes := output.Value + oldStake
				votesBytes, err := votes.Bytes()
				if err != nil {
					return err
				}
				c.Put(k, votesBytes)
			}

			// add vote to mempool
			c.mu.Lock()
			if p, ok := c.producerVotes[BytesToHexString(candidate)]; ok {
				if v, ok := p.Vote[vote.VoteType]; ok {
					c.producerVotes[BytesToHexString(candidate)].Vote[vote.VoteType] = v + output.Value
				} else {
					c.producerVotes[BytesToHexString(candidate)].Vote[vote.VoteType] = output.Value
				}
				c.dirty[vote.VoteType] = true
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
		for _, candidate := range vote.Candidates {
			// subtract vote to database
			key := []byte{byte(vote.VoteType)}
			k := append(key, candidate...)
			oldStake, err := c.getProducerVote(vote.VoteType, candidate)
			if err != nil {
				return nil
			} else {
				votes := oldStake - output.Value
				votesBytes, err := votes.Bytes()
				if err != nil {
					return err
				}
				c.Put(k, votesBytes)
			}

			// subtract vote to mempool
			c.mu.Lock()
			if p, ok := c.producerVotes[BytesToHexString(candidate)]; ok {
				if v, ok := p.Vote[vote.VoteType]; ok {
					c.producerVotes[BytesToHexString(candidate)].Vote[vote.VoteType] = v - output.Value
				}
				c.dirty[vote.VoteType] = true
			}
			c.mu.Unlock()
		}
	}

	return nil
}

func (c *ChainStore) ClearRegisteredProducer() {
	key := []byte{byte(DPOSVoteProducer)}
	c.BatchDelete(key)
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

func (c *ChainStore) getProducerVote(voteType outputpayload.VoteType, producer []byte) (Fixed64, error) {
	key := []byte{byte(voteType)}
	key = append(key, producer...)

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

func (c *ChainStore) PersistIllegalBlock(illegalBlocks *PayloadIllegalBlock, forceChange bool) error {
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

	if illegalBlocks.CoinType == ELACoin && forceChange {
		if err := DefaultLedger.Arbitrators.ForceChange(); err != nil {
			return err
		}
	}
	return nil
}

func (c *ChainStore) PersistIllegalProposal(payload *PayloadIllegalProposal) error {
	return c.persistIllegalPayload(func() []string {
		return []string{payload.Evidence.Proposal.Sponsor}
	})
}

func (c *ChainStore) PersistIllegalVote(payload *PayloadIllegalVote) error {
	return c.persistIllegalPayload(func() []string {
		return []string{payload.Evidence.Vote.Signer}
	})
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
