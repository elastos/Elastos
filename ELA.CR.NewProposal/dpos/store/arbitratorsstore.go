package store

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

type persistDutyChangedCountTask struct {
	count uint32
	reply chan bool
}

type persistCurrentArbitratorsTask struct {
	arbiters *Arbitrators
	reply    chan bool
}

type persistNextArbitratorsTask struct {
	arbiters *Arbitrators
	reply    chan bool
}

type persistDirectPeersTask struct {
	peers []*blockchain.DirectPeers
	reply chan bool
}

func (s *DposStore) arbiterLoop() {
	s.wg.Add(1)

out:
	for {
		select {
		case t := <-s.taskCh:
			now := time.Now()
			switch task := t.(type) {
			case *persistDutyChangedCountTask:
				s.handlePersistDposDutyChangedCount(task.count)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle dpos duty changed count exetime: %g", tcall)
			case *persistCurrentArbitratorsTask:
				s.handlePersistCurrentArbiters(task.arbiters)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle persist current arbiters exetime: %g", tcall)
			case *persistNextArbitratorsTask:
				s.handlePersistNextArbiters(task.arbiters)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle persist next arbiters exetime: %g", tcall)
			case *persistDirectPeersTask:
				s.handlePersistDirectPeers(task.peers)
				task.reply <- true
				tcall := float64(time.Now().Sub(now)) / float64(time.Second)
				log.Debugf("handle persist current arbiters exetime: %g", tcall)
			}

		case <-s.quit:
			break out
		}
	}

	s.wg.Done()
}

func (s *DposStore) StartRecordArbitrators() {
	go s.arbiterLoop()
}

func (s *DposStore) handlePersistDposDutyChangedCount(count uint32) {
	s.SaveDposDutyChangedCount(count)
}

func (s *DposStore) handlePersistCurrentArbiters(a *Arbitrators) {
	s.SaveCurrentArbitrators(a)
}

func (s *DposStore) handlePersistNextArbiters(a *Arbitrators) {
	s.SaveNextArbitrators(a)
}

func (s *DposStore) handlePersistDirectPeers(p []*blockchain.DirectPeers) {
	s.SaveDirectPeers(p)
}

func (s *DposStore) SaveDposDutyChangedCount(c uint32) {
	reply := make(chan bool)
	s.taskCh <- &persistDutyChangedCountTask{count: c, reply: reply}
	<-reply
}

func (s *DposStore) SaveCurrentArbitrators(a interfaces.Arbitrators) {
	reply := make(chan bool)
	if arbiters, ok := a.(*Arbitrators); ok {
		s.taskCh <- &persistCurrentArbitratorsTask{arbiters: arbiters, reply: reply}
		<-reply
	}
}

func (s *DposStore) SaveNextArbitrators(a interfaces.Arbitrators) {
	reply := make(chan bool)
	if arbiters, ok := a.(*Arbitrators); ok {
		s.taskCh <- &persistNextArbitratorsTask{arbiters: arbiters, reply: reply}
		<-reply
	}
}

func (s *DposStore) SaveDirectPeers(p []*blockchain.DirectPeers) {
	reply := make(chan bool)
	s.taskCh <- &persistDirectPeersTask{peers: p, reply: reply}
	<-reply
}

func (s *DposStore) GetArbitrators(a interfaces.Arbitrators) error {
	dutyChangedCount, err := s.getDposDutyChangedCount()
	if err != nil {
		return err
	}
	a.SetDutyChangedCount(dutyChangedCount)

	currentArbitrators, err := s.getCurrentArbitrators()
	if err != nil {
		return err
	}
	a.SetArbitrators(currentArbitrators)

	currentCandidates, err := s.getCurrentCandidates()
	if err != nil {
		return err
	}
	a.SetCandidates(currentCandidates)

	nextArbitrators, err := s.getNextArbitrators()
	if err != nil {
		return err
	}
	a.SetNextArbitrators(nextArbitrators)

	nextCandidates, err := s.getNextCandidates()
	if err != nil {
		return err
	}
	a.SetNextCandidates(nextCandidates)
	return nil
}

func (s *DposStore) GetDirectPeers() ([]*blockchain.DirectPeers, error) {
	key := []byte{byte(DPOSDirectPeers)}
	data, err := s.Get(key)
	if err != nil {
		return nil, err
	}

	var peers []*blockchain.DirectPeers
	r := bytes.NewReader(data)

	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	for i := uint64(0); i < count; i++ {
		publicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
		if err != nil {
			return nil, err
		}

		address, err := common.ReadVarString(r)
		if err != nil {
			return nil, err
		}

		sequence, err := common.ReadUint32(r)
		if err != nil {
			return nil, err
		}

		peers = append(peers, &blockchain.DirectPeers{
			PublicKey: publicKey,
			Address:   address,
			Sequence:  sequence,
		})
	}

	return peers, nil
}

func (s *DposStore) saveDposDutyChangedCount(count uint32) {
	log.Debugf("SaveDposDutyChangedCount()")
	batch := s.NewBatch()
	if err := s.persistDposDutyChangedCount(batch, count); err != nil {
		log.Fatal("[persistDposDutyChangedCount]: error to persist dpos duty changed count:", err.Error())
		return
	}
	batch.Commit()
}

func (s *DposStore) saveCurrentArbitrators(a *Arbitrators) {
	log.Debug("SaveCurrentArbitrators()")
	batch := s.NewBatch()
	if err := s.persistCurrentArbitrators(batch, a.CurrentArbitrators); err != nil {
		log.Fatal("[persistCurrentArbitrators]: error to persist current arbiters:", err.Error())
		return
	}
	if err := s.persistCurrentCandidates(batch, a.CurrentCandidates); err != nil {
		log.Fatal("[persistCurrentCandidates]: error to persist current candidates:", err.Error())
		return
	}
	batch.Commit()
}

func (s *DposStore) saveNextArbitrators(a *Arbitrators) {
	log.Debug("SaveNextArbitrators()")
	batch := s.NewBatch()
	if err := s.persistNextArbitrators(batch, a.NextArbitrators); err != nil {
		log.Fatal("[persistNextArbitrators]: error to persist current arbiters:", err.Error())
		return
	}
	if err := s.persistNextCandidates(batch, a.NextCandidates); err != nil {
		log.Fatal("[persistNextCandidates]: error to persist current candidates:", err.Error())
		return
	}
	batch.Commit()
}

func (s *DposStore) saveDirectPeers(p []*blockchain.DirectPeers) {
	log.Debug("SaveDirectPeers()")
	batch := s.NewBatch()
	if err := s.persistDirectPeers(batch, p); err != nil {
		log.Fatal("[persistDirectPeers]: error to persist direct peers:", err.Error())
		return
	}
	batch.Commit()
}
