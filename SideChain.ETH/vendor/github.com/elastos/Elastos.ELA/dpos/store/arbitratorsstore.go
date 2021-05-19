package store

import (
	"errors"
	"sort"

	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

func (s *DposStore) GetHeightsDesc() ([]uint32, error) {
	heights, err := s.getHeights()
	if err != nil {
		return nil, err
	}
	sort.Slice(heights, func(i, j int) bool {
		return heights[i] > heights[j]
	})
	return heights, err
}

func (s *DposStore) GetCheckPoint(height uint32) (*state.CheckPoint, error) {
	heights, err := s.GetHeightsDesc()
	if err != nil {
		return nil, err
	}

	for i := 0; i < len(heights); i++ {
		if height >= heights[i]+state.CheckPointInterval {
			return s.getCheckPointByHeight(heights[i])
		}
	}
	return nil, errors.New("can't find check point")
}

func (s *DposStore) getCheckPointByHeight(height uint32) (
	*state.CheckPoint, error) {
	var err error
	point := &state.CheckPoint{Height: height}

	var index uint32
	if index, err = s.getDutyIndex(point.Height); err != nil {
		return nil, err
	}
	point.DutyIndex = int(index)

	if point.CurrentArbitrators, err = s.getCurrentArbitrators(
		point.Height); err != nil {
		return nil, err
	}

	if point.CurrentCandidates, err = s.getCurrentCandidates(
		point.Height); err != nil {
		return nil, err
	}

	if point.NextArbitrators, err = s.getNextArbitrators(
		point.Height); err != nil {
		return nil, err
	}

	if point.NextCandidates, err = s.getNextCandidates(
		point.Height); err != nil {
		return nil, err
	}

	var reward *state.RewardData
	if reward, err = s.getCurrentReward(point.Height); err != nil {
		return nil, err
	}
	point.CurrentReward = *reward

	if reward, err = s.getNextReward(point.Height); err != nil {
		log.Warn("[SaveArbitersState] persistCurrentRewardData err: ", err)
		return nil, err
	}
	point.NextReward = *reward

	var frame *state.StateKeyFrame
	if frame, err = s.getStateKeyFrame(point.Height); err != nil {
		log.Warn("[SaveArbitersState] persistCurrentRewardData err: ", err)
		return nil, err
	}
	point.StateKeyFrame = *frame

	return point, nil
}

func (s *DposStore) SaveArbitersState(point *state.CheckPoint) (err error) {
	batch := s.db.NewBatch()

	if err = s.appendHeights(batch, point.Height); err != nil {
		log.Warn("[SaveArbitersState] appendHeights err: ", err)
		return
	}

	if err = s.persistDutyIndex(batch, point.Height, uint32(point.DutyIndex));
		err != nil {
		log.Warn("[SaveArbitersState] persistDutyIndex err: ", err)
		return
	}

	if err = s.persistCurrentArbitrators(batch, point.Height,
		point.CurrentArbitrators); err != nil {
		log.Warn("[SaveArbitersState] persistCurrentArbitrators err: ", err)
		return
	}

	if err = s.persistCurrentCandidates(batch, point.Height,
		point.CurrentCandidates); err != nil {
		log.Warn("[SaveArbitersState] persistCurrentCandidates err: ", err)
		return
	}

	if err = s.persistNextArbitrators(batch, point.Height,
		point.NextArbitrators); err != nil {
		log.Warn("[SaveArbitersState] persistNextArbitrators err: ", err)
		return
	}

	if err = s.persistNextCandidates(batch, point.Height,
		point.NextCandidates); err != nil {
		log.Warn("[SaveArbitersState] persistCurrentCandidates err: ", err)
		return
	}

	if err = s.persistCurrentRewardData(batch, point.Height,
		&point.CurrentReward); err != nil {
		log.Warn("[SaveArbitersState] persistCurrentRewardData err: ", err)
		return
	}

	if err = s.persistNextRewardData(batch, point.Height, &point.NextReward);
		err != nil {
		log.Warn("[SaveArbitersState] persistCurrentRewardData err: ", err)
		return
	}

	if err = s.persistStateKeyFrame(batch, point.Height, &point.StateKeyFrame);
		err != nil {
		log.Warn("[SaveArbitersState] persistStateKeyFrame err: ", err)
		return
	}

	if err = batch.Commit(); err != nil {
		log.Warn("[SaveArbitersState] batch commit err: ", err)
	}
	return
}
