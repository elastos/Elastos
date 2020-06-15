// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
)

func NewArbitratorsMock(arbitersByte []ArbiterMember, changeCount,
	majorityCount int) *ArbitratorsMock {
	return &ArbitratorsMock{
		CurrentArbitrators: arbitersByte,
		Snapshot: []*CheckPoint{
			{
				CurrentArbitrators: arbitersByte,
			},
		},
		CurrentCandidates:           make([]ArbiterMember, 0),
		CRCArbitrators:              make([]ArbiterMember, 0),
		NextArbitrators:             make([]ArbiterMember, 0),
		NextCandidates:              make([]ArbiterMember, 0),
		CurrentOwnerProgramHashes:   make([]*common.Uint168, 0),
		CandidateOwnerProgramHashes: make([]*common.Uint168, 0),
		OwnerVotesInRound:           make(map[common.Uint168]common.Fixed64),
		ArbitersRoundReward:         make(map[common.Uint168]common.Fixed64),
		CRCArbitratorsMap:           make(map[string]*Producer),
		ActiveProducer:              make([]ArbiterMember, 0),
		TotalVotesInRound:           0,
		DutyChangedCount:            0,
		MajorityCount:               majorityCount,
		FinalRoundChange:            0,
		InactiveMode:                false,
		CurrentReward:               *NewRewardData(),
		NextReward:                  *NewRewardData(),
	}
}

//mock object of arbitrators
type ArbitratorsMock struct {
	CurrentArbitrators          []ArbiterMember
	CRCArbitrators              []ArbiterMember
	CurrentCandidates           []ArbiterMember
	NextArbitrators             []ArbiterMember
	NextCandidates              []ArbiterMember
	CurrentOwnerProgramHashes   []*common.Uint168
	CandidateOwnerProgramHashes []*common.Uint168
	ArbitersRoundReward         map[common.Uint168]common.Fixed64
	OwnerVotesInRound           map[common.Uint168]common.Fixed64
	CRCArbitratorsMap           map[string]*Producer
	TotalVotesInRound           common.Fixed64
	DutyChangedCount            int
	MajorityCount               int
	FinalRoundChange            common.Fixed64
	InactiveMode                bool
	ActiveProducer              []ArbiterMember
	Snapshot                    []*CheckPoint
	CurrentReward               RewardData
	NextReward                  RewardData
}

func (a *ArbitratorsMock) GetConnectedProducer(publicKey []byte) ArbiterMember {
	panic("implement me")
}

func (a *ArbitratorsMock) CRCProducerCount() int {
	panic("implement me")
}

func (a *ArbitratorsMock) RecoverFromCheckPoints(height uint32) (uint32, error) {
	return height, nil
}

func (a *ArbitratorsMock) GetCurrentRewardData() RewardData {
	return a.CurrentReward
}

func (a *ArbitratorsMock) GetNextRewardData() RewardData {
	return a.NextReward
}

func (a *ArbitratorsMock) GetSnapshot(height uint32) []*CheckPoint {
	return a.Snapshot
}

func (a *ArbitratorsMock) IsActiveProducer(pk []byte) bool {
	for _, v := range a.ActiveProducer {
		if bytes.Equal(v.GetNodePublicKey(), pk) {
			return true
		}
	}
	return false
}

func (a *ArbitratorsMock) IsUnderstaffedMode() bool {
	return false
}

func (a *ArbitratorsMock) IsInactiveMode() bool {
	return a.InactiveMode
}

func (a *ArbitratorsMock) IsDisabledProducer(pk []byte) bool {
	return false
}

func (a *ArbitratorsMock) CheckDPOSIllegalTx(block *types.Block) error {
	return nil
}

func (a *ArbitratorsMock) GetArbitersRoundReward() map[common.Uint168]common.Fixed64 {
	return a.ArbitersRoundReward
}

func (a *ArbitratorsMock) GetFinalRoundChange() common.Fixed64 {
	return a.FinalRoundChange
}

func (a *ArbitratorsMock) Start() {
	panic("implement me")
}

func (a *ArbitratorsMock) GetDutyIndexByHeight(uint32) int {
	panic("implement me")
}

func (a *ArbitratorsMock) GetDutyIndex() int {
	panic("implement me")
}

func (a *ArbitratorsMock) ProcessSpecialTxPayload(p types.Payload, height uint32) error {
	panic("implement me")
}

func (a *ArbitratorsMock) CheckCRCAppropriationTx(block *types.Block) error {
	return nil
}

func (a *ArbitratorsMock) CheckNextTurnDPOSInfoTx(block *types.Block) error {
	return nil
}

func (a *ArbitratorsMock) GetNeedConnectArbiters() []peer.PID {
	panic("implement me")
}

func (a *ArbitratorsMock) IsNeedNextTurnDPOSInfo() bool {
	return false
}

func (a *ArbitratorsMock) IsArbitrator(pk []byte) bool {
	for _, v := range a.CurrentArbitrators {
		if bytes.Equal(v.GetNodePublicKey(), pk) {
			return true
		}
	}
	return false
}

func (a *ArbitratorsMock) IsCRCArbitrator(pk []byte) bool {
	for _, v := range a.CRCArbitrators {
		if bytes.Equal(v.GetNodePublicKey(), pk) {
			return true
		}
	}
	return false
}

func (a *ArbitratorsMock) GetArbitersCount() int {
	return len(a.CurrentArbitrators)
}

func (a *ArbitratorsMock) GetCRCArbitersCount() int {
	return len(a.CRCArbitrators)
}

func (a *ArbitratorsMock) GetArbitersMajorityCount() int {
	return a.MajorityCount
}

func (a *ArbitratorsMock) GetOnDutyCrossChainArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(0)
}

func (a *ArbitratorsMock) GetCrossChainArbitersMajorityCount() int {
	return a.MajorityCount
}

func (a *ArbitratorsMock) GetCrossChainArbitersCount() int {
	return len(a.CurrentArbitrators)
}

func (a *ArbitratorsMock) GetCrossChainArbiters() [][]byte {
	result := make([][]byte, 0, len(a.CurrentArbitrators))
	for _, v := range a.CurrentArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	return result
}

func (a *ArbitratorsMock) GetDutyChangeCount() int {
	return a.DutyChangedCount
}

func (a *ArbitratorsMock) IsNextCRCArbier(pk []byte) bool {
	return false
}

func (a *ArbitratorsMock) SetDutyChangeCount(count int) {
	a.DutyChangedCount = count
}

func (a *ArbitratorsMock) GetArbitrators() [][]byte {
	result := make([][]byte, 0, len(a.CurrentArbitrators))
	for _, v := range a.CurrentArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	return result
}

func (a *ArbitratorsMock) GetNormalArbitrators() ([][]byte, error) {
	result := make([][]byte, 0, len(a.CurrentArbitrators))
	for _, v := range a.CurrentArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	return result, nil
}

func (a *ArbitratorsMock) GetCandidates() [][]byte {
	result := make([][]byte, 0, len(a.CurrentCandidates))
	for _, v := range a.CurrentCandidates {
		result = append(result, v.GetNodePublicKey())
	}
	return result
}

func (a *ArbitratorsMock) GetNextArbitrators() [][]byte {
	result := make([][]byte, 0, len(a.NextArbitrators))
	for _, v := range a.NextArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	return result
}

func (a *ArbitratorsMock) GetNextCandidates() [][]byte {
	result := make([][]byte, 0, len(a.NextCandidates))
	for _, v := range a.NextCandidates {
		result = append(result, v.GetNodePublicKey())
	}
	return result
}

func (a *ArbitratorsMock) GetCRCArbiters() [][]byte {
	result := make([][]byte, 0, len(a.CRCArbitrators))
	for _, v := range a.CRCArbitrators {
		result = append(result, v.GetNodePublicKey())
	}
	return result
}

func (a *ArbitratorsMock) GetDutyChangedCount() int {
	return a.DutyChangedCount
}

func (a *ArbitratorsMock) SetDutyChangedCount(count int) {
	a.DutyChangedCount = count
}

func (a *ArbitratorsMock) SetArbitrators(ar []ArbiterMember) {
	a.CurrentArbitrators = ar
}

func (a *ArbitratorsMock) SetCandidates(ca []ArbiterMember) {
	a.CurrentCandidates = ca
}

func (a *ArbitratorsMock) SetNextArbitrators(ar []ArbiterMember) {
	a.NextArbitrators = ar
}

func (a *ArbitratorsMock) SetNextCandidates(ca []ArbiterMember) {
	a.NextCandidates = ca
}

func (a *ArbitratorsMock) GetOnDutyArbitrator() []byte {
	return a.GetNextOnDutyArbitrator(0)
}

func (a *ArbitratorsMock) GetNextOnDutyArbitrator(offset uint32) []byte {
	if len(a.CurrentArbitrators) == 0 {
		return nil
	}
	index := (a.DutyChangedCount + int(offset)) % len(a.CurrentArbitrators)
	return a.CurrentArbitrators[index].GetNodePublicKey()
}

func (a *ArbitratorsMock) HasArbitersMajorityCount(num int) bool {
	return num > a.MajorityCount
}

func (a *ArbitratorsMock) HasArbitersMinorityCount(num int) bool {
	return num >= len(a.CurrentArbitrators)-a.MajorityCount
}

func (a *ArbitratorsMock) DumpInfo(height uint32) {
}
