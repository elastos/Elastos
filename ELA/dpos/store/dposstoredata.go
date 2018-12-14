package store

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

func (s *DposStore) getDposDutyChangedCount() (uint32, error) {
	key := []byte{byte(DPOSDutyChangedCount)}
	data, err := s.Get(key)
	if err == nil {
		result, err := common.ReadUint32(bytes.NewReader(data))
		if err != nil {
			return 0, err
		}
		return result, nil
	}

	return 0, nil
}

func (s *DposStore) getCurrentArbitrators() ([][]byte, error) {
	var currentArbitrators [][]byte
	key := []byte{byte(DPOSCurrentArbitrators)}
	data, err := s.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			arbiter, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "arbiter")
			if err != nil {
				return nil, err
			}
			currentArbitrators = append(currentArbitrators, arbiter)
		}
	}

	return currentArbitrators, nil
}

func (s *DposStore) getCurrentCandidates() ([][]byte, error) {
	var currentCandidates [][]byte
	key := []byte{byte(DPOSCurrentCandidates)}
	data, err := s.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			candidate, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "candidate")
			if err != nil {
				return nil, err
			}
			currentCandidates = append(currentCandidates, candidate)
		}
	}

	return currentCandidates, nil
}

func (s *DposStore) getNextArbitrators() ([][]byte, error) {
	var nextArbitrators [][]byte
	key := []byte{byte(DPOSNextArbitrators)}
	data, err := s.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			arbiter, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "arbiter")
			if err != nil {
				return nil, err
			}
			nextArbitrators = append(nextArbitrators, arbiter)
		}
	}
	return nextArbitrators, nil
}

func (s *DposStore) getNextCandidates() ([][]byte, error) {
	var nextCandidates [][]byte
	key := []byte{byte(DPOSNextCandidates)}
	data, err := s.Get(key)
	if err == nil {

		r := bytes.NewReader(data)
		count, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}

		for i := uint64(0); i < count; i++ {
			candidate, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "candidate")
			if err != nil {
				return nil, err
			}
			nextCandidates = append(nextCandidates, candidate)
		}
	}

	return nextCandidates, nil
}

func (s *DposStore) persistDposDutyChangedCount(batch Batch, count uint32) error {
	key := []byte{byte(DPOSDutyChangedCount)}

	value := new(bytes.Buffer)
	common.WriteUint32(value, count)

	batch.Put(key, value.Bytes())
	return nil
}

func (s *DposStore) persistCurrentArbitrators(batch Batch, arbiters [][]byte) error {
	return s.persistBytesArray(batch, arbiters, DPOSCurrentArbitrators)
}

func (s *DposStore) persistCurrentCandidates(batch Batch, candidates [][]byte) error {
	return s.persistBytesArray(batch, candidates, DPOSCurrentCandidates)
}

func (s *DposStore) persistNextArbitrators(batch Batch, arbiters [][]byte) error {
	return s.persistBytesArray(batch, arbiters, DPOSNextArbitrators)
}

func (s *DposStore) persistNextCandidates(batch Batch, candidates [][]byte) error {
	return s.persistBytesArray(batch, candidates, DPOSNextCandidates)
}

func (s *DposStore) persistBytesArray(batch Batch, bytesArray [][]byte, prefix DataEntryPrefix) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(prefix))

	value := new(bytes.Buffer)
	if err := common.WriteUint64(value, uint64(len(bytesArray))); err != nil {
		return err
	}

	for _, b := range bytesArray {
		if err := common.WriteVarBytes(value, b); err != nil {
			return err
		}
	}

	batch.Put(key.Bytes(), value.Bytes())
	return nil
}

func (s *DposStore) persistDirectPeers(batch Batch, peers []*interfaces.DirectPeers) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DPOSDirectPeers))

	value := new(bytes.Buffer)

	if err := common.WriteVarUint(value, uint64(len(peers))); err != nil {
		return err
	}

	for _, p := range peers {
		if err := common.WriteVarBytes(value, p.PublicKey); err != nil {
			return err
		}

		if err := common.WriteVarString(value, p.Address); err != nil {
			return err
		}

		if err := common.WriteUint32(value, p.Sequence); err != nil {
			return err
		}
	}

	batch.Put(key.Bytes(), value.Bytes())
	return nil
}
