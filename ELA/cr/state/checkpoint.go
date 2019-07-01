package state

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"
)

// CRMember defines CR committee member related info
type CRMember struct {
	Info             payload.CRInfo
	ImpeachmentVotes common.Fixed64
}

// StateKeyFrame holds necessary state about CR committee
type KeyFrame struct {
	Members             []*CRMember
	LastCommitteeHeight uint32
}

// StateKeyFrame holds necessary state about CR state
type StateKeyFrame struct {
	CodeDIDMap         map[string]common.Uint168
	PendingCandidates  map[common.Uint168]*Candidate
	ActivityCandidates map[common.Uint168]*Candidate
	CanceledCandidates map[common.Uint168]*Candidate
	Nicknames          map[string]struct{}
}

func (c *CRMember) Serialize(w io.Writer) (err error) {
	if err = c.Info.SerializeUnsigned(w, payload.CRInfoVersion); err != nil {
		return
	}

	return common.WriteUint64(w, uint64(c.ImpeachmentVotes))
}

func (c *CRMember) Deserialize(r io.Reader) (err error) {
	if err = c.Info.DeserializeUnsigned(r, payload.CRInfoVersion); err != nil {
		return
	}

	var votes uint64
	if votes, err = common.ReadUint64(r); err != nil {
		return
	}

	c.ImpeachmentVotes = common.Fixed64(votes)
	return
}

func (k *KeyFrame) Serialize(w io.Writer) (err error) {
	if err = common.WriteVarUint(w, uint64(len(k.Members))); err != nil {
		return
	}

	for _, v := range k.Members {
		if err = v.Serialize(w); err != nil {
			return
		}
	}

	return common.WriteUint32(w, k.LastCommitteeHeight)
}

func (k *KeyFrame) Deserialize(r io.Reader) (err error) {
	var memLen uint64
	if memLen, err = common.ReadVarUint(r, 0); err != nil {
		return
	}

	k.Members = make([]*CRMember, 0, memLen)
	for i := uint64(0); i < memLen; i++ {
		member := &CRMember{}
		if err = member.Deserialize(r); err != nil {
			return
		}
		k.Members = append(k.Members, member)
	}

	k.LastCommitteeHeight, err = common.ReadUint32(r)
	return
}

func (k *KeyFrame) Snapshot() *KeyFrame {
	frame := NewKeyFrame()
	frame.LastCommitteeHeight = k.LastCommitteeHeight
	frame.Members = k.Members
	return frame
}

func NewKeyFrame() *KeyFrame {
	return &KeyFrame{
		Members:             make([]*CRMember, 0),
		LastCommitteeHeight: 0,
	}
}

func (k *StateKeyFrame) Serialize(w io.Writer) (err error) {
	if err = k.SerializeCodeAddressMap(w, k.CodeDIDMap); err != nil {
		return
	}

	if err = k.SerializeCandidateMap(w, k.PendingCandidates); err != nil {
		return
	}

	if err = k.SerializeCandidateMap(w, k.ActivityCandidates); err != nil {
		return
	}

	if err = k.SerializeCandidateMap(w, k.CanceledCandidates); err != nil {
		return
	}

	if err = utils.SerializeStringSet(w, k.Nicknames); err != nil {
		return
	}

	return
}

func (k *StateKeyFrame) Deserialize(r io.Reader) (err error) {
	if k.CodeDIDMap, err = k.DeserializeCodeAddressMap(r); err != nil {
		return
	}

	if k.PendingCandidates, err = k.DeserializeCandidateMap(r); err != nil {
		return
	}

	if k.ActivityCandidates, err = k.DeserializeCandidateMap(r); err != nil {
		return
	}

	if k.CanceledCandidates, err = k.DeserializeCandidateMap(r); err != nil {
		return
	}

	if k.Nicknames, err = utils.DeserializeStringSet(r); err != nil {
		return
	}
	return
}

func (k *StateKeyFrame) SerializeCodeAddressMap(w io.Writer,
	cmap map[string]common.Uint168) (err error) {
	if err = common.WriteVarUint(w, uint64(len(cmap))); err != nil {
		return
	}
	for k, v := range cmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (k *StateKeyFrame) DeserializeCodeAddressMap(r io.Reader) (
	cmap map[string]common.Uint168, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	cmap = make(map[string]common.Uint168)

	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		var v common.Uint168
		if err = v.Deserialize(r); err != nil {
			return
		}
		cmap[k] = v
	}
	return
}

func (k *StateKeyFrame) SerializeCandidateMap(w io.Writer,
	cmap map[common.Uint168]*Candidate) (err error) {
	if err = common.WriteVarUint(w, uint64(len(cmap))); err != nil {
		return
	}
	for k, v := range cmap {
		if err = k.Serialize(w); err != nil {
			return
		}

		if err = v.Serialize(w); err != nil {
			return
		}
	}
	return
}

func (k *StateKeyFrame) DeserializeCandidateMap(
	r io.Reader) (cmap map[common.Uint168]*Candidate, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	cmap = make(map[common.Uint168]*Candidate)
	for i := uint64(0); i < count; i++ {
		var k common.Uint168
		if err = k.Deserialize(r); err != nil {
			return
		}
		candidate := &Candidate{}
		if err = candidate.Deserialize(r); err != nil {
			return
		}
		cmap[k] = candidate
	}
	return
}

// Snapshot will create a new StateKeyFrame object and deep copy all related data.
func (k *StateKeyFrame) Snapshot() *StateKeyFrame {
	state := NewStateKeyFrame()
	state.CodeDIDMap = copyCodeAddressMap(k.CodeDIDMap)
	state.PendingCandidates = copyCandidateMap(k.PendingCandidates)
	state.ActivityCandidates = copyCandidateMap(k.ActivityCandidates)
	state.CanceledCandidates = copyCandidateMap(k.CanceledCandidates)
	state.Nicknames = utils.CopyStringSet(k.Nicknames)

	return state
}

func NewStateKeyFrame() *StateKeyFrame {
	return &StateKeyFrame{
		CodeDIDMap:         make(map[string]common.Uint168),
		PendingCandidates:  make(map[common.Uint168]*Candidate),
		ActivityCandidates: make(map[common.Uint168]*Candidate),
		CanceledCandidates: make(map[common.Uint168]*Candidate),
		Nicknames:          make(map[string]struct{}),
	}
}

// copyCandidateMap copy the CR map's key and value, and return the dst map.
func copyCandidateMap(src map[common.Uint168]*Candidate) (
	dst map[common.Uint168]*Candidate) {
	dst = map[common.Uint168]*Candidate{}
	for k, v := range src {
		p := *v
		dst[k] = &p
	}
	return
}

// copyCodeAddressMap copy the map's key and value, and return the dst map.
func copyCodeAddressMap(src map[string]common.Uint168) (
	dst map[string]common.Uint168) {
	dst = map[string]common.Uint168{}
	for k, v := range src {
		dst[k] = v
	}
	return
}

func copyCRMembers(src []*CRMember) []*CRMember {
	dst := make([]*CRMember, 0, len(src))
	for _, v := range src {
		m := *v
		dst = append(dst, &m)
	}
	return dst
}
