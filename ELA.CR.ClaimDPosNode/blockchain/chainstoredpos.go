package blockchain

import (
	"bytes"
	"errors"
	"sort"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
)

const ProducerConfirmations = 6

func (c *ChainStore) GetRegisteredProducers() []*PayloadRegisterProducer {
	c.mu.Lock()
	defer c.mu.Unlock()

	illProducers := c.getIllegalProducers()

	result := make([]*PayloadRegisterProducer, 0)

	for _, p := range c.producerVotes {
		if _, ok := illProducers[BytesToHexString(p.Payload.OwnPublicKey)]; ok {
			continue
		}
		result = append(result, p.Payload)
	}

	return result
}

func (c *ChainStore) GetActiveRegisteredProducers() []*PayloadRegisterProducer {
	c.mu.Lock()
	defer c.mu.Unlock()

	illProducers := c.getIllegalProducers()

	result := make([]*PayloadRegisterProducer, 0)

	for _, p := range c.producerVotes {
		if _, ok := illProducers[BytesToHexString(p.Payload.OwnPublicKey)]; ok {
			continue
		}
		if c.currentBlockHeight-p.RegHeight+1 < ProducerConfirmations {
			continue
		}
		result = append(result, p.Payload)
	}

	return result
}

func (c *ChainStore) GetRegisteredProducersSorted() ([]*PayloadRegisterProducer, error) {
	c.mu.Lock()
	defer c.mu.Unlock()

	if dirty, ok := c.dirty[outputpayload.Delegate]; ok && dirty {
		producersInfo := make([]*ProducerInfo, 0)
		for _, v := range c.producerVotes {
			producersInfo = append(producersInfo, v)
		}
		if len(producersInfo) == 0 {
			return nil, errors.New("[GetRegisteredProducers] not found producer")
		}

		sort.Slice(producersInfo, func(i, j int) bool {
			ivalue := producersInfo[i].Vote
			jvalue := producersInfo[j].Vote
			if ivalue == jvalue {
				return bytes.Compare(producersInfo[i].Payload.OwnPublicKey, producersInfo[j].Payload.OwnPublicKey) > 0
			}
			return ivalue > jvalue
		})

		producers := make([]*PayloadRegisterProducer, 0)
		illProducers := c.getIllegalProducers()
		for _, p := range producersInfo {
			if _, ok := illProducers[BytesToHexString(p.Payload.OwnPublicKey)]; ok {
				continue
			}
			producers = append(producers, p.Payload)
		}

		c.orderedProducers = producers
		c.dirty[outputpayload.Delegate] = false
	}

	return c.orderedProducers, nil
}

func (c *ChainStore) GetProducerVote(publicKey []byte) Fixed64 {
	c.mu.Lock()
	defer c.mu.Unlock()

	info, ok := c.producerVotes[BytesToHexString(publicKey)]
	if !ok {
		return Fixed64(0)
	}

	return info.Vote
}

func (c *ChainStore) GetProducerStatus(publicKey string) ProducerState {
	c.mu.Lock()
	defer c.mu.Unlock()

	if p, ok := c.producerVotes[publicKey]; ok {
		if c.currentBlockHeight-p.RegHeight+1 >= ProducerConfirmations {
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

	height, ok := c.canceledProducers[BytesToHexString(publicKey)]
	if !ok {
		return 0, errors.New("not found canceled producer")
	}
	return height, nil
}
