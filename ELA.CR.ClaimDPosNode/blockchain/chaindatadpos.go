package blockchain

import (
	"bytes"
	"errors"

	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

func (c *ChainStore) PersistRegisterProducer(payload *PayloadRegisterProducer) error {
	key := []byte{byte(DPOSVoteProducer)}
	hBuf := new(bytes.Buffer)
	height := c.GetHeight()
	err := WriteUint32(hBuf, height)
	if err != nil {
		return errors.New("write height failed")
	}
	producerBytes, err := c.getRegisteredProducers()
	if err != nil {
		count := new(bytes.Buffer)
		err = WriteUint64(count, uint64(1))
		if err != nil {
			return errors.New("write count failed")
		}
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
		if bytes.Equal(p.PublicKey, payload.PublicKey) {
			return errors.New("duplicated public key")
		}
	}

	// PUT VALUE: length(uint64),oldProducers(height+payload),newProducer
	value := new(bytes.Buffer)
	err = WriteUint64(value, length+uint64(1))
	if err != nil {
		return errors.New("write new count failed")
	}
	c.BatchPut(key, append(append(value.Bytes(), producerBytes[8:]...),
		append(hBuf.Bytes(), payload.Data(PayloadRegisterProducerVersion)...)...))

	return c.recordProducer(payload, height)
}

func (c *ChainStore) RollbackRegisterProducer(payload *PayloadRegisterProducer) error {
	return c.PersistCancelProducer(&PayloadCancelProducer{PublicKey: payload.PublicKey})
}

func (c *ChainStore) recordProducer(payload *PayloadRegisterProducer, regHeight uint32) error {
	programHash, err := contract.PublicKeyToStandardProgramHash(payload.PublicKey)
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
	for _, t := range outputpayload.VoteTypes {
		c.dirty[t] = true
	}
	return nil
}

func (c *ChainStore) PersistCancelProducer(payload *PayloadCancelProducer) error {
	key := []byte{byte(DPOSVoteProducer)}
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
		if !bytes.Equal(p.PublicKey, payload.PublicKey) {
			buf := new(bytes.Buffer)
			err := WriteUint32(buf, h)
			if err != nil {
				return errors.New("write height failed")
			}
			p.Serialize(buf, PayloadRegisterProducerVersion)
			newProducerBytes = append(newProducerBytes, buf.Bytes()...)
			count++
		}
	}

	value := new(bytes.Buffer)
	err = WriteUint64(value, count)
	if err != nil {
		return errors.New("write count failed")
	}
	newProducerBytes = append(value.Bytes(), newProducerBytes...)

	c.BatchPut(key, newProducerBytes)

	programHash, err := contract.PublicKeyToStandardProgramHash(payload.PublicKey)
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
	// update producer in database
	key := []byte{byte(DPOSVoteProducer)}
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
		if bytes.Equal(p.PublicKey, payload.PublicKey) {
			pld = payload
		} else {
			pld = &p
		}
		buf := new(bytes.Buffer)
		err = WriteUint32(buf, h)
		if err != nil {
			return errors.New("write height failed")
		}
		pld.Serialize(buf, PayloadRegisterProducerVersion)
		newProducerBytes = append(newProducerBytes, buf.Bytes()...)
	}

	value := new(bytes.Buffer)
	err = WriteUint64(value, length)
	if err != nil {
		return errors.New("write count failed")
	}
	newProducerBytes = append(value.Bytes(), newProducerBytes...)

	c.BatchPut(key, newProducerBytes)

	programHash, err := contract.PublicKeyToStandardProgramHash(payload.PublicKey)
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
				c.dirty[vote.VoteType] = true
			}
			c.mu.Unlock()
		}
	}

	return nil
}

func (c *ChainStore) getRegisteredProducers() ([]byte, error) {
	key := []byte{byte(DPOSVoteProducer)}
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

func (c *ChainStore) getDposDutyChangedCount() (uint32, error) {
	key := []byte{byte(DPOSDutyChangedCount)}
	data, err := c.Get(key)
	if err == nil {
		result, err := ReadUint32(bytes.NewReader(data))
		if err != nil {
			return 0, err
		}
		return result, nil
	}

	return 0, nil
}

func (c *ChainStore) getCurrentArbitrators() ([][]byte, error) {
	var currentArbitrators [][]byte
	key := []byte{byte(DPOSCurrentArbitrators)}
	data, err := c.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			arbiter, err := ReadVarBytes(r, crypto.NegativeBigLength, "arbiter")
			if err != nil {
				return nil, err
			}
			currentArbitrators = append(currentArbitrators, arbiter)
		}
	}

	return currentArbitrators, nil
}

func (c *ChainStore) getCurrentCandidates() ([][]byte, error) {
	var currentCandidates [][]byte
	key := []byte{byte(DPOSCurrentCandidates)}
	data, err := c.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			candidate, err := ReadVarBytes(r, crypto.NegativeBigLength, "candidate")
			if err != nil {
				return nil, err
			}
			currentCandidates = append(currentCandidates, candidate)
		}
	}

	return currentCandidates, nil
}

func (c *ChainStore) getNextArbitrators() ([][]byte, error) {
	var nextArbitrators [][]byte
	key := []byte{byte(DPOSNextArbitrators)}
	data, err := c.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			arbiter, err := ReadVarBytes(r, crypto.NegativeBigLength, "arbiter")
			if err != nil {
				return nil, err
			}
			nextArbitrators = append(nextArbitrators, arbiter)
		}
	}
	return nextArbitrators, nil
}

func (c *ChainStore) getNextCandidates() ([][]byte, error) {
	var nextCandidates [][]byte
	key := []byte{byte(DPOSNextCandidates)}
	data, err := c.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			candidate, err := ReadVarBytes(r, crypto.NegativeBigLength, "candidate")
			if err != nil {
				return nil, err
			}
			nextCandidates = append(nextCandidates, candidate)
		}
	}

	return nextCandidates, nil
}

func (c *ChainStore) persistDposDutyChangedCount(count uint32) error {
	key := []byte{byte(DPOSDutyChangedCount)}

	value := new(bytes.Buffer)
	WriteUint32(value, count)

	c.BatchPut(key, value.Bytes())
	return nil
}

func (c *ChainStore) persistCurrentArbitrators(arbiters [][]byte) error {
	return c.persistBytesArray(arbiters, DPOSCurrentArbitrators)
}

func (c *ChainStore) persistCurrentCandidates(candidates [][]byte) error {
	return c.persistBytesArray(candidates, DPOSCurrentCandidates)
}

func (c *ChainStore) persistNextArbitrators(arbiters [][]byte) error {
	return c.persistBytesArray(arbiters, DPOSNextArbitrators)
}

func (c *ChainStore) persistNextCandidates(candidates [][]byte) error {
	return c.persistBytesArray(candidates, DPOSNextCandidates)
}

func (c *ChainStore) persistBytesArray(bytesArray [][]byte, prefix DataEntryPrefix) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(prefix))

	value := new(bytes.Buffer)
	if err := WriteUint64(value, uint64(len(bytesArray))); err != nil {
		return err
	}

	for _, b := range bytesArray {
		if err := WriteVarBytes(value, b); err != nil {
			return err
		}
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}

func (c *ChainStore) persistDirectPeers(peers []*DirectPeers) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DPOSDirectPeers))

	value := new(bytes.Buffer)

	if err := WriteVarUint(value, uint64(len(peers))); err != nil {
		return err
	}

	for _, p := range peers {
		if err := WriteVarBytes(value, p.PublicKey); err != nil {
			return err
		}

		if err := WriteVarString(value, p.Address); err != nil {
			return err
		}

		if err := WriteUint32(value, p.Sequence); err != nil {
			return err
		}
	}

	c.BatchPut(key.Bytes(), value.Bytes())
	return nil
}
