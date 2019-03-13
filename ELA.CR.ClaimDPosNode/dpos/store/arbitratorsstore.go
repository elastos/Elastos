package store

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

type persistDutyChangedCountTask struct {
	count uint32
	reply chan bool
}

type persistDirectPeersTask struct {
	peers []*DirectPeers
	reply chan bool
}

func (s *DposStore) arbiterLoop() {
	s.wg.Add(1)

out:
	for {
		select {
		case t := <-s.persistCh:
			switch task := t.(type) {
			case *persistDutyChangedCountTask:
				s.handlePersistDposDutyChangedCount(task.count)
				task.reply <- true
			case *persistDirectPeersTask:
				s.handlePersistDirectPeers(task.peers)
				task.reply <- true
			}

		case <-s.quit:
			break out
		}
	}

	s.wg.Done()
}

func (s *DposStore) StartArbitratorsRecord() {
	go s.arbiterLoop()
}

func (s *DposStore) handlePersistDposDutyChangedCount(count uint32) {
	s.saveDposDutyChangedCount(count)
}

func (s *DposStore) handlePersistDirectPeers(p []*DirectPeers) {
	s.saveDirectPeers(p)
}

func (s *DposStore) SaveDposDutyChangedCount(c uint32) {
	reply := make(chan bool)
	s.persistCh <- &persistDutyChangedCountTask{count: c, reply: reply}
	<-reply
}

func (s *DposStore) SaveDirectPeers(p []*DirectPeers) {
	reply := make(chan bool)
	s.persistCh <- &persistDirectPeersTask{peers: p, reply: reply}
	<-reply
}

func (s *DposStore) GetDirectPeers() ([]*DirectPeers, error) {
	key := []byte{byte(DPOSDirectPeers)}
	data, err := s.db.Get(key)
	if err != nil {
		return nil, err
	}

	var peers []*DirectPeers
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

		peers = append(peers, &DirectPeers{
			PublicKey: publicKey,
			Address:   address,
			Sequence:  sequence,
		})
	}

	return peers, nil
}

func (s *DposStore) saveDposDutyChangedCount(count uint32) {
	log.Debugf("SaveDposDutyChangedCount()")
	batch := s.db.NewBatch()
	if err := s.persistDposDutyChangedCount(batch, count); err != nil {
		log.Fatal("[persistDposDutyChangedCount]: error to persist dpos duty changed count:", err.Error())
		return
	}
	batch.Commit()
}

func (s *DposStore) saveDirectPeers(p []*DirectPeers) {
	log.Debug("SaveDirectPeers()")
	batch := s.db.NewBatch()
	if err := s.persistDirectPeers(batch, p); err != nil {
		log.Fatal("[persistDirectPeers]: error to persist direct peers:", err.Error())
		return
	}
	batch.Commit()
}
