package store

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"

	"github.com/syndtr/goleveldb/leveldb/errors"
)

func (s *DposStore) getDutyIndex(height uint32) (uint32, error) {
	key, err := s.getKey(height, DPOSDutyIndex)
	if err != nil {
		return 0, err
	}

	data, err := s.db.Get(key)
	if err == nil {
		result, err := common.ReadUint32(bytes.NewReader(data))
		if err != nil {
			return 0, err
		}
		return result, nil
	}

	return 0, nil
}

func (s *DposStore) getCurrentArbitrators(height uint32) ([][]byte, error) {
	key, err := s.getKey(height, DPOSCurrentArbitrators)
	if err != nil {
		return nil, err
	}

	var currentArbitrators [][]byte
	data, err := s.db.Get(key)
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

func (s *DposStore) getCurrentCandidates(height uint32) ([][]byte, error) {
	key, err := s.getKey(height, DPOSCurrentCandidates)
	if err != nil {
		return nil, err
	}

	var currentCandidates [][]byte
	data, err := s.db.Get(key)
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

func (s *DposStore) getNextArbitrators(height uint32) ([][]byte, error) {
	key, err := s.getKey(height, DPOSNextArbitrators)

	var nextArbitrators [][]byte
	data, err := s.db.Get(key)
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

func (s *DposStore) getNextCandidates(height uint32) ([][]byte, error) {
	key, err := s.getKey(height, DPOSNextCandidates)
	if err != nil {
		return nil, err
	}

	var nextCandidates [][]byte
	data, err := s.db.Get(key)
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

func (s *DposStore) getCurrentReward(height uint32) (*state.RewardData, error) {
	return s.getRewardData(height, DPOSCurrentReward)
}

func (s *DposStore) getNextReward(height uint32) (*state.RewardData, error) {
	return s.getRewardData(height, DPOSNextReward)
}

func (s *DposStore) getKey(height uint32, prefix DataEntryPrefix) ([]byte,
	error) {
	key := new(bytes.Buffer)
	key.WriteByte(byte(prefix))
	if err := common.WriteUint32(key, height); err != nil {
		return nil, err
	}
	return key.Bytes(), nil
}

func (s *DposStore) getHeights() ([]uint32, error) {
	key := []byte{byte(DPOSCheckPoints)}

	data, err := s.db.Get(key)
	if err != nil {
		if err == errors.ErrNotFound {
			return make([]uint32, 0), nil
		}
		return nil, err
	}

	r := bytes.NewReader(data)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}
	heights := make([]uint32, 0, count+1)
	var h uint32
	for i := uint64(0); i < count; i++ {
		if h, err = common.ReadUint32(r); err != nil {
			return nil, err
		}
		heights = append(heights, h)
	}
	return heights, nil
}

func (s *DposStore) appendHeights(batch Batch, height uint32) error {
	heights, err := s.getHeights()
	if err != nil {
		return err
	}
	heights = append(heights, height)
	heightSet := make(map[uint32]interface{})
	for _, v := range heights {
		heightSet[v] = nil
	}

	key := []byte{byte(DPOSCheckPoints)}
	buf := new(bytes.Buffer)
	if err = common.WriteVarUint(buf, uint64(len(heightSet))); err != nil {
		return err
	}

	for h := range heightSet {
		if err = common.WriteUint32(buf, h); err != nil {
			return err
		}
	}
	return batch.Put(key, buf.Bytes())
}

func (s *DposStore) getStateKeyFrame(height uint32) (*state.StateKeyFrame,
	error) {
	key, err := s.getKey(height, DPOSState)
	if err != nil {
		return nil, err
	}

	data, err := s.db.Get(key)
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(data)
	frame := &state.StateKeyFrame{}
	if err = frame.Deserialize(r); err != nil {
		return nil, err
	}
	return frame, nil
}

func (s *DposStore) persistStateKeyFrame(batch Batch, height uint32,
	frame *state.StateKeyFrame) error {
	key, err := s.getKey(height, DPOSState)
	if err != nil {
		return err
	}

	value := new(bytes.Buffer)
	if err := frame.Serialize(value); err != nil {
		return err
	}

	return batch.Put(key, value.Bytes())
}

func (s *DposStore) persistDutyIndex(batch Batch, height uint32,
	count uint32) error {
	key, err := s.getKey(height, DPOSDutyIndex)
	if err != nil {
		return err
	}

	value := new(bytes.Buffer)
	if err := common.WriteUint32(value, count); err != nil {
		return err
	}

	return batch.Put(key, value.Bytes())
}

func (s *DposStore) persistCurrentArbitrators(batch Batch, height uint32,
	arbiters [][]byte) error {
	return s.persistBytesArray(batch, height, arbiters, DPOSCurrentArbitrators)
}

func (s *DposStore) persistCurrentCandidates(batch Batch, height uint32,
	candidates [][]byte) error {
	return s.persistBytesArray(batch, height, candidates, DPOSCurrentCandidates)
}

func (s *DposStore) persistNextArbitrators(batch Batch, height uint32,
	arbiters [][]byte) error {
	return s.persistBytesArray(batch, height, arbiters, DPOSNextArbitrators)
}

func (s *DposStore) persistNextCandidates(batch Batch, height uint32,
	candidates [][]byte) error {
	return s.persistBytesArray(batch, height, candidates, DPOSNextCandidates)
}

func (s *DposStore) persistCurrentRewardData(batch Batch, height uint32,
	rewards *state.RewardData) error {
	return s.persistRewardData(batch, height, rewards, DPOSCurrentReward)
}

func (s *DposStore) persistNextRewardData(batch Batch, height uint32,
	rewards *state.RewardData) error {
	return s.persistRewardData(batch, height, rewards, DPOSNextReward)
}

func (s *DposStore) persistBytesArray(batch Batch, height uint32,
	bytesArray [][]byte, prefix DataEntryPrefix) error {
	key, err := s.getKey(height, prefix)
	if err != nil {
		return err
	}

	value := new(bytes.Buffer)
	if err := common.WriteVarUint(value, uint64(len(bytesArray))); err != nil {
		return err
	}

	for _, b := range bytesArray {
		if err := common.WriteVarBytes(value, b); err != nil {
			return err
		}
	}

	return batch.Put(key, value.Bytes())
}

func (s *DposStore) getRewardData(height uint32,
	prefix DataEntryPrefix) (*state.RewardData, error) {

	key, err := s.getKey(height, prefix)
	if err != nil {
		return nil, err
	}
	data, err := s.db.Get(key)
	if err != nil {
		return nil, err
	}

	rewards := state.NewRewardData()
	r := bytes.NewReader(data)
	if err = rewards.Deserialize(r); err != nil {
		return nil, err
	}

	return rewards, nil
}

func (s *DposStore) persistRewardData(batch Batch, height uint32,
	rewards *state.RewardData, prefix DataEntryPrefix) error {
	key, err := s.getKey(height, prefix)
	if err != nil {
		return err
	}

	value := new(bytes.Buffer)
	if err = rewards.Serialize(value); err != nil {
		return err
	}

	return batch.Put(key, value.Bytes())
}
