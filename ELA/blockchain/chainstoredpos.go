package blockchain

import (
	"bytes"
	"errors"
	"sort"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
)

type producerSorter []*ProducerInfo

func (s producerSorter) Len() int {
	return len(s)
}

func (s producerSorter) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func (s producerSorter) Less(i, j int) bool {
	ivalue, _ := s[i].Vote[currentVoteType]
	jvalue, _ := s[j].Vote[currentVoteType]
	if ivalue == jvalue {
		return bytes.Compare(s[i].Payload.PublicKey, s[j].Payload.PublicKey) > 0
	}
	return ivalue > jvalue
}

func (c *ChainStore) GetRegisteredProducers() []*PayloadRegisterProducer {
	c.mu.Lock()
	defer c.mu.Unlock()

	illProducers := c.getIllegalProducers()

	result := make([]*PayloadRegisterProducer, 0)

	for _, p := range c.producerVotes {
		if _, ok := illProducers[BytesToHexString(p.Payload.PublicKey)]; ok {
			continue
		}
		result = append(result, p.Payload)
	}

	return result
}

func (c *ChainStore) GetRegisteredProducersByVoteType(voteType outputpayload.VoteType) ([]*PayloadRegisterProducer, error) {
	c.mu.Lock()
	defer c.mu.Unlock()

	if dirty, ok := c.dirty[voteType]; ok && dirty {
		producersInfo := make([]*ProducerInfo, 0)
		for _, v := range c.producerVotes {
			producersInfo = append(producersInfo, v)
		}
		if len(producersInfo) == 0 {
			return nil, errors.New("[GetRegisteredProducers] not found producer")
		}

		currentVoteType = voteType
		sort.Sort(producerSorter(producersInfo))

		producers := make([]*PayloadRegisterProducer, 0)
		illProducers := c.getIllegalProducers()
		for _, p := range producersInfo {
			if _, ok := illProducers[BytesToHexString(p.Payload.PublicKey)]; ok {
				continue
			}
			producers = append(producers, p.Payload)
		}

		c.orderedProducers[voteType] = producers
		c.dirty[voteType] = false
	}

	if result, ok := c.orderedProducers[voteType]; ok {
		return result, nil
	}

	return nil, errors.New("[GetRegisteredProducers] not found vote")
}

func (c *ChainStore) GetProducerVote(voteType outputpayload.VoteType, publicKey []byte) Fixed64 {
	c.mu.Lock()
	defer c.mu.Unlock()

	info, ok := c.producerVotes[BytesToHexString(publicKey)]
	if !ok {
		return Fixed64(0)
	}

	vote, ok := info.Vote[voteType]
	if !ok {
		return Fixed64(0)
	}

	return vote
}

func (c *ChainStore) GetProducerStatus(address string) ProducerState {
	c.mu.Lock()
	defer c.mu.Unlock()

	if p, ok := c.producerVotes[address]; ok {
		if c.currentBlockHeight-p.RegHeight >= 6 {
			return ProducerRegistered
		} else {
			return ProducerRegistering
		}
	}
	return ProducerUnRegistered
}

func (c *ChainStore) GetIllegalProducers() map[string]struct{} {
	c.mu.Lock()
	defer c.mu.Unlock()

	return c.getIllegalProducers()
}

func (c *ChainStore) GetCancelProducerHeight(publicKey []byte) (uint32, error) {
	c.mu.Lock()
	defer c.mu.Unlock()

	return c.getCancelProducerHeight(publicKey)
}
