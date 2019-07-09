// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package store

import (
	"bytes"
	"os"
	"path/filepath"
	"strconv"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/syndtr/goleveldb/leveldb/errors"
)

const flatCheckPointExtension = ".cp"

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
	key := []byte{byte(DPOSCheckPointHeights)}

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

	key := []byte{byte(DPOSCheckPointHeights)}
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

func (s *DposStore) getSingleCheckPoint(
	height uint32) (*state.CheckPoint, error) {
	key, err := s.getKey(height, DPOSSingleCheckPoint)
	if err != nil {
		return nil, err
	}

	data, err := s.db.Get(key)
	r := bytes.NewReader(data)

	point := &state.CheckPoint{}
	if err = point.Deserialize(r); err != nil {
		return nil, err
	}
	return point, nil
}

func (s *DposStore) persistSingleCheckPoint(batch Batch, height uint32,
	point *state.CheckPoint) error {
	key, err := s.getKey(height, DPOSSingleCheckPoint)
	if err != nil {
		return err
	}

	value := new(bytes.Buffer)
	if err := point.Serialize(value); err != nil {
		return err
	}

	return batch.Put(key, value.Bytes())
}

func (s *DposStore) saveFlatCheckPoint(point *state.CheckPoint) error {
	fileName := filepath.Join(s.dataDir, "dpos", strconv.FormatUint(uint64(
		point.Height), 10)+flatCheckPointExtension)
	file, err := os.OpenFile(fileName,
		os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600)
	if err != nil {
		return err
	}

	if err = point.Serialize(file); err != nil {
		return err
	}
	return file.Close()
}

func (s *DposStore) getFlatCheckPoint(height uint32) (*state.CheckPoint,
	error) {
	fileName := filepath.Join(s.dataDir, "dpos",
		strconv.FormatUint(uint64(height), 10)+flatCheckPointExtension)
	file, err := os.OpenFile(fileName, os.O_RDONLY, 0400)
	defer file.Close()
	if err != nil {
		return nil, errors.New("open check point file failed")
	}

	point := &state.CheckPoint{}
	if err = point.Deserialize(file); err != nil {
		return nil, err
	}
	return point, nil
}
