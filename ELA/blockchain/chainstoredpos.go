package blockchain

import (
	"errors"
	"sort"
	"strings"

	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	. "github.com/elastos/Elastos.ELA/core/types/payload"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/common/log"
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
		return strings.Compare(s[i].Payload.PublicKey, s[j].Payload.PublicKey) > 0
	}
	return ivalue > jvalue
}

func (c *ChainStore) GetRegisteredProducers() []*PayloadRegisterProducer {
	c.mu.Lock()
	defer c.mu.Unlock()

	illProducers := c.getIllegalProducers()

	result := make([]*PayloadRegisterProducer, 0)

	for _, p := range c.producerVotes {
		if _, ok := illProducers[p.Payload.PublicKey]; ok {
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
			if _, ok := illProducers[p.Payload.PublicKey]; ok {
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

func (c *ChainStore) GetProducerVote(voteType outputpayload.VoteType, programHash Uint168) Fixed64 {
	c.mu.Lock()
	defer c.mu.Unlock()

	info, ok := c.producerVotes[programHash]
	if !ok {
		return Fixed64(0)
	}

	vote, ok := info.Vote[voteType]
	if !ok {
		return Fixed64(0)
	}

	return vote
}

func (c *ChainStore) GetProducerStatus(programHash Uint168) ProducerState {
	c.mu.Lock()
	defer c.mu.Unlock()

	if p, ok := c.producerVotes[programHash]; ok {
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

func (c *ChainStore) GetArbitrators(a *arbitrators) error {
	var err error
	if a.dutyChangedCount, err = c.getDposDutyChangedCount(); err != nil {
		return err
	}

	if a.currentArbitrators, err = c.getCurrentArbitrators(); err != nil {
		return err
	}

	if a.currentCandidates, err = c.getCurrentCandidates(); err != nil {
		return err
	}

	if a.nextArbitrators, err = c.getNextArbitrators(); err != nil {
		return err
	}

	if a.nextCandidates, err = c.getNextCandidates(); err != nil {
		return err
	}
	return nil
}

func (c *ChainStore) SaveDposDutyChangedCount(count uint32) {
	log.Debug("SaveDposDutyChangedCount()")

	reply := make(chan bool)
	c.taskCh <- &persistDutyChangedCountTask{count: count, reply: reply}
	<-reply
}

func (c *ChainStore) SaveCurrentArbitrators(a *arbitrators) {
	log.Debug("SaveCurrentArbitrators()")

	reply := make(chan bool)
	c.taskCh <- &persistCurrentArbitratorsTask{arbiters: a, reply: reply}
	<-reply
}

func (c *ChainStore) SaveNextArbitrators(a *arbitrators) {
	log.Debug("SaveNextArbitrators()")

	reply := make(chan bool)
	c.taskCh <- &persistNextArbitratorsTask{arbiters: a, reply: reply}
	<-reply
}

func (c *ChainStore) handlePersistDposDutyChangedCount(count uint32) {
	c.NewBatch()
	if err := c.persistDposDutyChangedCount(count); err != nil {
		log.Fatal("[persistDposDutyChangedCount]: error to persist dpos duty changed count:", err.Error())
		return
	}
	c.BatchCommit()
}

func (c *ChainStore) handlePersistCurrentArbiters(a *arbitrators) {
	c.NewBatch()
	if err := c.persistCurrentArbitrators(a.currentArbitrators); err != nil {
		log.Fatal("[persistCurrentArbitrators]: error to persist current arbiters:", err.Error())
		return
	}
	if err := c.persistCurrentCandidates(a.currentCandidates); err != nil {
		log.Fatal("[persistCurrentCandidates]: error to persist current candidates:", err.Error())
		return
	}
	c.BatchCommit()
}

func (c *ChainStore) handlePersistNextArbiters(a *arbitrators) {
	c.NewBatch()
	if err := c.persistNextArbitrators(a.nextArbitrators); err != nil {
		log.Fatal("[persistNextArbitrators]: error to persist current arbiters:", err.Error())
		return
	}
	if err := c.persistNextCandidates(a.nextCandidates); err != nil {
		log.Fatal("[persistNextCandidates]: error to persist current candidates:", err.Error())
		return
	}
	c.BatchCommit()
}
