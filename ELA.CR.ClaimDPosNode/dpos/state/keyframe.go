// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

// StateKeyFrame holds necessary state about State
type StateKeyFrame struct {
	NodeOwnerKeys            map[string]string // NodePublicKey as key, OwnerPublicKey as value
	PendingProducers         map[string]*Producer
	ActivityProducers        map[string]*Producer
	InactiveProducers        map[string]*Producer
	CanceledProducers        map[string]*Producer
	IllegalProducers         map[string]*Producer
	PendingCanceledProducers map[string]*Producer
	Votes                    map[string]struct{}
	DepositOutputs           map[string]common.Fixed64
	Nicknames                map[string]struct{}
	SpecialTxHashes          map[common.Uint256]struct{}
	PreBlockArbiters         map[string]struct{}
	ProducerDepositMap       map[common.Uint168]struct{}

	EmergencyInactiveArbiters map[string]struct{}
	VersionStartHeight        uint32
	VersionEndHeight          uint32
	NeedNextTurnDposInfo      bool
}

// RewardData defines variables to calculate reward of a round
type RewardData struct {
	OwnerVotesInRound map[common.Uint168]common.Fixed64
	TotalVotesInRound common.Fixed64
}

// snapshot takes a snapshot of current state and returns the copy.
func (s *StateKeyFrame) snapshot() *StateKeyFrame {
	state := StateKeyFrame{
		NodeOwnerKeys:            make(map[string]string),
		PendingProducers:         make(map[string]*Producer),
		ActivityProducers:        make(map[string]*Producer),
		InactiveProducers:        make(map[string]*Producer),
		CanceledProducers:        make(map[string]*Producer),
		IllegalProducers:         make(map[string]*Producer),
		PendingCanceledProducers: make(map[string]*Producer),
		Votes:                    make(map[string]struct{}),
		DepositOutputs:           make(map[string]common.Fixed64),
		Nicknames:                make(map[string]struct{}),
		SpecialTxHashes:          make(map[common.Uint256]struct{}),
		PreBlockArbiters:         make(map[string]struct{}),
		ProducerDepositMap:       make(map[common.Uint168]struct{}),
	}
	state.NodeOwnerKeys = copyStringMap(s.NodeOwnerKeys)
	state.PendingProducers = copyProducerMap(s.PendingProducers)
	state.ActivityProducers = copyProducerMap(s.ActivityProducers)
	state.InactiveProducers = copyProducerMap(s.InactiveProducers)
	state.CanceledProducers = copyProducerMap(s.CanceledProducers)
	state.IllegalProducers = copyProducerMap(s.IllegalProducers)
	state.PendingCanceledProducers = copyProducerMap(s.PendingCanceledProducers)
	state.Votes = copyStringSet(s.Votes)
	state.DepositOutputs = copyFixed64Map(s.DepositOutputs)
	state.Nicknames = copyStringSet(s.Nicknames)
	state.SpecialTxHashes = copyHashSet(s.SpecialTxHashes)
	state.PreBlockArbiters = copyStringSet(s.PreBlockArbiters)
	state.ProducerDepositMap = copyDIDSet(s.ProducerDepositMap)
	return &state
}

func (s *StateKeyFrame) Serialize(w io.Writer) (err error) {

	if err = s.SerializeStringMap(s.NodeOwnerKeys, w); err != nil {
		return
	}

	if err = s.SerializeProducerMap(s.PendingProducers, w); err != nil {
		return
	}

	if err = s.SerializeProducerMap(s.ActivityProducers, w); err != nil {
		return
	}

	if err = s.SerializeProducerMap(s.InactiveProducers, w); err != nil {
		return
	}

	if err = s.SerializeProducerMap(s.CanceledProducers, w); err != nil {
		return
	}

	if err = s.SerializeProducerMap(s.IllegalProducers, w); err != nil {
		return
	}

	if err = s.SerializeProducerMap(s.PendingCanceledProducers, w); err != nil {
		return
	}

	if err = s.SerializeStringSet(s.Votes, w); err != nil {
		return
	}

	if err = s.SerializeFixed64Map(s.DepositOutputs, w); err != nil {
		return
	}

	if err = s.SerializeStringSet(s.Nicknames, w); err != nil {
		return
	}

	if err = s.SerializeHashSet(s.SpecialTxHashes, w); err != nil {
		return
	}

	if err = s.SerializeStringSet(s.PreBlockArbiters, w); err != nil {
		return
	}

	if err = s.SerializeDIDSet(s.ProducerDepositMap, w); err != nil {
		return
	}

	if err = s.SerializeStringSet(s.EmergencyInactiveArbiters, w); err != nil {
		return
	}

	if err = common.WriteUint32(w, s.VersionStartHeight); err != nil {
		return
	}

	return common.WriteUint32(w, s.VersionEndHeight)
}

func (s *StateKeyFrame) Deserialize(r io.Reader) (err error) {
	if s.NodeOwnerKeys, err = s.DeserializeStringMap(r); err != nil {
		return
	}

	if s.PendingProducers, err = s.DeserializeProducerMap(r); err != nil {
		return
	}

	if s.ActivityProducers, err = s.DeserializeProducerMap(r); err != nil {
		return
	}

	if s.InactiveProducers, err = s.DeserializeProducerMap(r); err != nil {
		return
	}

	if s.CanceledProducers, err = s.DeserializeProducerMap(r); err != nil {
		return
	}

	if s.IllegalProducers, err = s.DeserializeProducerMap(r); err != nil {
		return
	}

	if s.PendingCanceledProducers, err = s.DeserializeProducerMap(r); err != nil {
		return
	}

	if s.Votes, err = s.DeserializeStringSet(r); err != nil {
		return
	}

	if s.DepositOutputs, err = s.DeserializeFixed64Map(r); err != nil {
		return
	}

	if s.Nicknames, err = s.DeserializeStringSet(r); err != nil {
		return
	}

	if s.SpecialTxHashes, err = s.DeserializeHashSet(r); err != nil {
		return
	}

	if s.PreBlockArbiters, err = s.DeserializeStringSet(r); err != nil {
		return
	}

	if s.ProducerDepositMap, err = s.DeserializeDIDSet(r); err != nil {
		return
	}

	if s.EmergencyInactiveArbiters, err = s.DeserializeStringSet(r); err != nil {
		return
	}

	if s.VersionStartHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if s.VersionEndHeight, err = common.ReadUint32(r); err != nil {
		return
	}
	return
}

func (s *StateKeyFrame) SerializeHashSet(vmap map[common.Uint256]struct{},
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k := range vmap {
		if err = k.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (s *StateKeyFrame) DeserializeHashSet(
	r io.Reader) (vmap map[common.Uint256]struct{}, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[common.Uint256]struct{})
	for i := uint64(0); i < count; i++ {
		k := common.Uint256{}
		if err = k.Deserialize(r); err != nil {
			return
		}
		vmap[k] = struct{}{}
	}
	return
}

func (s *StateKeyFrame) SerializeFixed64Map(vmap map[string]common.Fixed64,
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k, v := range vmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}
		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (s *StateKeyFrame) DeserializeFixed64Map(
	r io.Reader) (vmap map[string]common.Fixed64, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[string]common.Fixed64)
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		var v common.Fixed64
		if err = v.Deserialize(r); err != nil {
			return
		}
		vmap[k] = v
	}
	return
}

func (s *StateKeyFrame) SerializeStringSet(vmap map[string]struct{},
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k := range vmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}
	}
	return
}

func (s *StateKeyFrame) DeserializeStringSet(
	r io.Reader) (vmap map[string]struct{}, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[string]struct{})
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		vmap[k] = struct{}{}
	}
	return
}

func (s *StateKeyFrame) SerializeDIDSet(vmap map[common.Uint168]struct{},
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k := range vmap {
		if err = k.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (s *StateKeyFrame) DeserializeDIDSet(
	r io.Reader) (vmap map[common.Uint168]struct{}, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[common.Uint168]struct{})
	for i := uint64(0); i < count; i++ {
		k := common.Uint168{}
		if err = k.Deserialize(r); err != nil {
			return
		}
		vmap[k] = struct{}{}
	}
	return
}

func (s *StateKeyFrame) SerializeStringMap(smap map[string]string,
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(smap))); err != nil {
		return
	}
	for k, v := range smap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}

		if err = common.WriteVarString(w, v); err != nil {
			return
		}
	}
	return
}

func (s *StateKeyFrame) DeserializeStringMap(
	r io.Reader) (smap map[string]string, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	smap = make(map[string]string)
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		var v string
		if v, err = common.ReadVarString(r); err != nil {
			return
		}
		smap[k] = v
	}
	return
}

func (s *StateKeyFrame) SerializeProducerMap(pmap map[string]*Producer,
	w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(pmap))); err != nil {
		return
	}
	for k, v := range pmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (s *StateKeyFrame) DeserializeProducerMap(
	r io.Reader) (pmap map[string]*Producer, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	pmap = make(map[string]*Producer)
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		producer := &Producer{}
		if err = producer.Deserialize(r); err != nil {
			return
		}
		pmap[k] = producer
	}
	return
}

func NewStateKeyFrame() *StateKeyFrame {
	return &StateKeyFrame{
		NodeOwnerKeys:             make(map[string]string),
		PendingProducers:          make(map[string]*Producer),
		ActivityProducers:         make(map[string]*Producer),
		InactiveProducers:         make(map[string]*Producer),
		CanceledProducers:         make(map[string]*Producer),
		IllegalProducers:          make(map[string]*Producer),
		PendingCanceledProducers:  make(map[string]*Producer),
		Votes:                     make(map[string]struct{}),
		DepositOutputs:            make(map[string]common.Fixed64),
		Nicknames:                 make(map[string]struct{}),
		SpecialTxHashes:           make(map[common.Uint256]struct{}),
		PreBlockArbiters:          make(map[string]struct{}),
		EmergencyInactiveArbiters: make(map[string]struct{}),
		ProducerDepositMap:        make(map[common.Uint168]struct{}),
		VersionStartHeight:        0,
		VersionEndHeight:          0,
	}
}

func (d *RewardData) Serialize(w io.Writer) error {
	if err := common.WriteUint64(w, uint64(d.TotalVotesInRound)); err != nil {
		return err
	}

	if err := common.WriteVarUint(w,
		uint64(len(d.OwnerVotesInRound))); err != nil {
		return err
	}
	for k, v := range d.OwnerVotesInRound {
		if err := k.Serialize(w); err != nil {
			return err
		}
		if err := common.WriteUint64(w, uint64(v)); err != nil {
			return err
		}
	}
	return nil
}

func (d *RewardData) Deserialize(r io.Reader) (err error) {
	var votes uint64
	if votes, err = common.ReadUint64(r); err != nil {
		return
	}
	d.TotalVotesInRound = common.Fixed64(votes)

	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	d.OwnerVotesInRound = make(map[common.Uint168]common.Fixed64)
	for i := uint64(0); i < count; i++ {
		k := common.Uint168{}
		if err = k.Deserialize(r); err != nil {
			return
		}
		var v uint64
		if v, err = common.ReadUint64(r); err != nil {
			return
		}
		d.OwnerVotesInRound[k] = common.Fixed64(v)
	}
	return
}

func NewRewardData() *RewardData {
	return &RewardData{
		OwnerVotesInRound: make(map[common.Uint168]common.Fixed64),
		TotalVotesInRound: 0,
	}
}

// copyProducerMap copy the src map's key, value pairs into dst map.
func copyProducerMap(src map[string]*Producer) (dst map[string]*Producer) {
	dst = map[string]*Producer{}
	for k, v := range src {
		p := *v
		dst[k] = &p
	}
	return
}

func copyStringMap(src map[string]string) (dst map[string]string) {
	dst = map[string]string{}
	for k, v := range src {
		p := v
		dst[k] = p
	}
	return
}

func copyFixed64Map(src map[string]common.Fixed64) (dst map[string]common.Fixed64) {
	dst = map[string]common.Fixed64{}
	for k, v := range src {
		p := v
		dst[k] = p
	}
	return
}

func copyStringSet(src map[string]struct{}) (dst map[string]struct{}) {
	dst = map[string]struct{}{}
	for k := range src {
		dst[k] = struct{}{}
	}
	return
}

func copyHashSet(src map[common.Uint256]struct{}) (
	dst map[common.Uint256]struct{}) {
	dst = map[common.Uint256]struct{}{}
	for k := range src {
		dst[k] = struct{}{}
	}
	return
}

func copyDIDSet(src map[common.Uint168]struct{}) (
	dst map[common.Uint168]struct{}) {
	dst = map[common.Uint168]struct{}{}
	for k := range src {
		dst[k] = struct{}{}
	}
	return
}

func copyByteList(src []ArbiterMember) (dst []ArbiterMember) {
	for _, v := range src {
		dst = append(dst, v.Clone())
	}
	return
}

func copyReward(src *RewardData) (dst *RewardData) {
	dst = &RewardData{
		OwnerVotesInRound: make(map[common.Uint168]common.Fixed64),
	}
	dst.TotalVotesInRound = src.TotalVotesInRound

	for k, v := range src.OwnerVotesInRound {
		dst.OwnerVotesInRound[k] = v
	}
	return
}

func copyCRCArbitersMap(src map[common.Uint168]ArbiterMember) (dst map[common.Uint168]ArbiterMember) {
	dst = make(map[common.Uint168]ArbiterMember)
	for k, v := range src {
		member := v.Clone()
		dst[k] = member
	}

	return dst
}
