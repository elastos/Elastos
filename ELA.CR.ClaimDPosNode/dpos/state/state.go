// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"io"
	"math"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/utils"
)

// ProducerState represents the state of a producer.
type ProducerState byte

const (
	// Pending indicates the producer is just registered and didn't get 6
	// confirmations yet.
	Pending ProducerState = iota

	// Active indicates the producer is registered and confirmed by more than
	// 6 blocks.
	Active

	// Inactive indicates the producer has been inactivated for a period which shall
	// be punished and will be activated later.
	Inactive

	// Canceled indicates the producer was canceled.
	Canceled

	// Illegal indicates the producer was found to break the consensus.
	Illegal

	// Returned indicates the producer has canceled and deposit returned.
	Returned
)

// CacheVotesSize indicate the size to cache votes information.
const CacheVotesSize = 6

// producerStateStrings is a array of producer states back to their constant
// names for pretty printing.
var producerStateStrings = []string{"Pending", "Active", "Inactive",
	"Canceled", "Illegal", "Returned"}

func (ps ProducerState) String() string {
	if int(ps) < len(producerStateStrings) {
		return producerStateStrings[ps]
	}
	return fmt.Sprintf("ProducerState-%d", ps)
}

// Producer holds a producer's info.  It provides read only methods to access
// producer's info.
type Producer struct {
	info                   payload.ProducerInfo
	state                  ProducerState
	registerHeight         uint32
	cancelHeight           uint32
	inactiveCountingHeight uint32
	inactiveSince          uint32
	activateRequestHeight  uint32
	illegalHeight          uint32
	penalty                common.Fixed64
	votes                  common.Fixed64
	depositAmount          common.Fixed64
	depositHash            common.Uint168
}

// Info returns a copy of the origin registered producer info.
func (p *Producer) Info() payload.ProducerInfo {
	return p.info
}

// State returns the producer's state, can be pending, active or canceled.
func (p *Producer) State() ProducerState {
	return p.state
}

// RegisterHeight returns the height when the producer was registered.
func (p *Producer) RegisterHeight() uint32 {
	return p.registerHeight
}

// CancelHeight returns the height when the producer was canceled.
func (p *Producer) CancelHeight() uint32 {
	return p.cancelHeight
}

// Votes returns the votes of the producer.
func (p *Producer) Votes() common.Fixed64 {
	return p.votes
}

func (p *Producer) NodePublicKey() []byte {
	return p.info.NodePublicKey
}

func (p *Producer) OwnerPublicKey() []byte {
	return p.info.OwnerPublicKey
}

func (p *Producer) Penalty() common.Fixed64 {
	return p.penalty
}

func (p *Producer) InactiveSince() uint32 {
	return p.inactiveSince
}

func (p *Producer) IllegalHeight() uint32 {
	return p.illegalHeight
}

func (p *Producer) ActivateRequestHeight() uint32 {
	return p.activateRequestHeight
}

func (p *Producer) DepositAmount() common.Fixed64 {
	return p.depositAmount
}

func (p *Producer) Serialize(w io.Writer) error {
	if err := p.info.Serialize(w, payload.ProducerInfoVersion); err != nil {
		return err
	}

	if err := common.WriteUint8(w, uint8(p.state)); err != nil {
		return err
	}

	if err := common.WriteUint32(w, p.registerHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, p.cancelHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, p.inactiveCountingHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, p.inactiveSince); err != nil {
		return err
	}

	if err := common.WriteUint32(w, p.activateRequestHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, p.illegalHeight); err != nil {
		return err
	}

	if err := common.WriteUint64(w, uint64(p.penalty)); err != nil {
		return err
	}

	if err := common.WriteUint64(w, uint64(p.votes)); err != nil {
		return err
	}

	return p.depositHash.Serialize(w)
}

func (p *Producer) Deserialize(r io.Reader) (err error) {
	if err = p.info.Deserialize(r, payload.ProducerInfoVersion); err != nil {
		return
	}

	var state uint8
	if state, err = common.ReadUint8(r); err != nil {
		return
	}
	p.state = ProducerState(state)

	if p.registerHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if p.cancelHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if p.inactiveCountingHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if p.inactiveSince, err = common.ReadUint32(r); err != nil {
		return
	}

	if p.activateRequestHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	if p.illegalHeight, err = common.ReadUint32(r); err != nil {
		return
	}

	var penalty uint64
	if penalty, err = common.ReadUint64(r); err != nil {
		return
	}
	p.penalty = common.Fixed64(penalty)

	var votes uint64
	if votes, err = common.ReadUint64(r); err != nil {
		return
	}
	p.votes = common.Fixed64(votes)

	return p.depositHash.Deserialize(r)
}

const (
	// maxHistoryCapacity indicates the maximum capacity of change history.
	maxHistoryCapacity = 10

	// ActivateDuration is about how long we should activate from pending or
	// inactive state
	ActivateDuration = 6
)

// State is a memory database storing DPOS producers state, like pending
// producers active producers and their votes.
type State struct {
	*StateKeyFrame

	// getArbiters defines methods about get current arbiters
	getArbiters              func() [][]byte
	getCRMembers             func() []*state.CRMember
	getProducerDepositAmount func(programHash common.Uint168) (
		common.Fixed64, error)
	getTxReference func(tx *types.Transaction) (
		map[*types.Input]types.Output, error)
	chainParams *config.Params

	mtx     sync.RWMutex
	history *utils.History
}

// getProducerKey returns the producer's owner public key string, whether the
// given public key is the producer's node public key or owner public key.
func (s *State) getProducerKey(publicKey []byte) string {
	key := hex.EncodeToString(publicKey)

	// If the given public key is node public key, get the producer's owner
	// public key.
	if owner, ok := s.NodeOwnerKeys[key]; ok {
		return owner
	}

	return key
}

// getProducer returns a producer with the producer's node public key or it's
// owner public key, if no matches return nil.
func (s *State) getProducer(publicKey []byte) *Producer {
	key := s.getProducerKey(publicKey)
	return s.getProducerByOwnerPublicKey(key)
}

// getProducer returns a producer with the producer's owner public key,
// if no matches return nil.
func (s *State) getProducerByOwnerPublicKey(key string) *Producer {
	if producer, ok := s.ActivityProducers[key]; ok {
		return producer
	}
	if producer, ok := s.CanceledProducers[key]; ok {
		return producer
	}
	if producer, ok := s.IllegalProducers[key]; ok {
		return producer
	}
	if producer, ok := s.PendingProducers[key]; ok {
		return producer
	}
	if producer, ok := s.InactiveProducers[key]; ok {
		return producer
	}
	return nil
}

// updateProducerInfo updates the producer's info with value compare, any change
// will be updated.
func (s *State) updateProducerInfo(origin *payload.ProducerInfo, update *payload.ProducerInfo) {
	producer := s.getProducer(origin.OwnerPublicKey)

	// compare and update node nickname.
	if origin.NickName != update.NickName {
		delete(s.Nicknames, origin.NickName)
		s.Nicknames[update.NickName] = struct{}{}
	}

	// compare and update node public key, we only query pending and active node
	// because canceled and illegal node can not be updated.
	if !bytes.Equal(origin.NodePublicKey, update.NodePublicKey) {
		oldKey := hex.EncodeToString(origin.NodePublicKey)
		newKey := hex.EncodeToString(update.NodePublicKey)
		delete(s.NodeOwnerKeys, oldKey)
		s.NodeOwnerKeys[newKey] = hex.EncodeToString(origin.OwnerPublicKey)
	}

	producer.info = *update
}

func (s *State) ExistProducerByDepositHash(programHash common.Uint168) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	_, ok := s.ProducerDepositMap[programHash]
	return ok
}

// GetProducer returns a producer with the producer's node public key or it's
// owner public key including canceled and illegal producers.  If no matches
// return nil.
func (s *State) GetProducer(publicKey []byte) *Producer {
	s.mtx.RLock()
	producer := s.getProducer(publicKey)
	s.mtx.RUnlock()
	return producer
}

// GetProducers returns all producers including pending and active producers (no
// canceled and illegal producers).
func (s *State) GetProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.PendingProducers)+
		len(s.ActivityProducers))
	for _, producer := range s.PendingProducers {
		producers = append(producers, producer)
	}
	for _, producer := range s.ActivityProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetAllProducers returns all producers including pending, active, canceled, illegal and inactive producers.
func (s *State) GetAllProducers() []*Producer {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	return s.getAllProducers()
}

func (s *State) getAllProducers() []*Producer {
	producers := make([]*Producer, 0, len(s.PendingProducers)+
		len(s.ActivityProducers))
	for _, producer := range s.PendingProducers {
		producers = append(producers, producer)
	}
	for _, producer := range s.ActivityProducers {
		producers = append(producers, producer)
	}
	for _, producer := range s.InactiveProducers {
		producers = append(producers, producer)
	}
	for _, producer := range s.CanceledProducers {
		producers = append(producers, producer)
	}
	for _, producer := range s.IllegalProducers {
		producers = append(producers, producer)
	}
	return producers
}

// GetPendingProducers returns all producers that in pending state.
func (s *State) GetPendingProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.PendingProducers))
	for _, producer := range s.PendingProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetActiveProducers returns all producers that in active state.
func (s *State) GetActiveProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.ActivityProducers))
	for _, producer := range s.ActivityProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetVotedProducers returns all producers that in active state with votes.
func (s *State) GetVotedProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.ActivityProducers))
	for _, producer := range s.ActivityProducers {
		// limit arbiters can only be producers who have votes
		if producer.Votes() > 0 {
			producers = append(producers, producer)
		}
	}
	s.mtx.RUnlock()
	return producers
}

// GetCanceledProducers returns all producers that in cancel state.
func (s *State) GetCanceledProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.CanceledProducers))
	for _, producer := range s.CanceledProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetPendingCanceledProducers returns all producers that in pending canceled state.
func (s *State) GetPendingCanceledProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.PendingCanceledProducers))
	for _, producer := range s.PendingCanceledProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetReturnedDepositProducers returns producers that in returned deposit state.
func (s *State) GetReturnedDepositProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.CanceledProducers))
	for _, producer := range s.CanceledProducers {
		if producer.state == Returned {
			producers = append(producers, producer)
		}
	}
	s.mtx.RUnlock()
	return producers
}

// GetIllegalProducers returns all illegal producers.
func (s *State) GetIllegalProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.IllegalProducers))
	for _, producer := range s.IllegalProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetIllegalProducers returns all inactive producers.
func (s *State) GetInactiveProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.InactiveProducers))
	for _, producer := range s.InactiveProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// IsPendingProducer returns if a producer is in pending list according to the
// public key.
func (s *State) IsPendingProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.PendingProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsActiveProducer returns if a producer is in activate list according to the
// public key.
func (s *State) IsActiveProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.ActivityProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsInactiveProducer returns if a producer is in inactivate list according to
// the public key.
func (s *State) IsInactiveProducer(publicKey []byte) bool {
	s.mtx.RLock()
	ok := s.isInactiveProducer(publicKey)
	s.mtx.RUnlock()
	return ok
}

func (s *State) isInactiveProducer(publicKey []byte) bool {
	_, ok := s.InactiveProducers[s.getProducerKey(publicKey)]
	return ok
}

// IsCanceledProducer returns if a producer is in canceled list according to the
// public key.
func (s *State) IsCanceledProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.CanceledProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsIllegalProducer returns if a producer is in illegal list according to the
// public key.
func (s *State) IsIllegalProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.IllegalProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsAbleToRecoverFromInactiveMode returns if most of the emergency arbiters have activated
// and able to work again
func (s *State) IsAbleToRecoverFromInactiveMode() bool {
	activatedNum := 0

	s.mtx.RLock()
	totalNum := len(s.EmergencyInactiveArbiters)
	for k := range s.EmergencyInactiveArbiters {
		if _, ok := s.InactiveProducers[k]; !ok {
			activatedNum++
		}
	}
	s.mtx.RUnlock()

	return totalNum == 0 || float64(activatedNum)/float64(totalNum) >
		MajoritySignRatioNumerator/MajoritySignRatioDenominator
}

// IsAbleToRecoverFromInactiveMode returns if there are enough active arbiters
func (s *State) IsAbleToRecoverFromUnderstaffedState() bool {
	s.mtx.RLock()
	result := len(s.ActivityProducers) >= s.chainParams.GeneralArbiters
	s.mtx.RUnlock()
	return result
}

// LeaveEmergency will reset EmergencyInactiveArbiters variable
func (s *State) LeaveEmergency(history *utils.History, height uint32) {
	s.mtx.Lock()
	oriArbiters := s.EmergencyInactiveArbiters
	history.Append(height, func() {
		s.EmergencyInactiveArbiters = map[string]struct{}{}
	}, func() {
		s.EmergencyInactiveArbiters = oriArbiters
	})
	s.mtx.Unlock()
}

// NicknameExists returns if a nickname is exists.
func (s *State) NicknameExists(nickname string) bool {
	s.mtx.RLock()
	_, ok := s.Nicknames[nickname]
	s.mtx.RUnlock()
	return ok
}

// ProducerExists returns if a producer is exists by it's node public key or
// owner public key.
func (s *State) ProducerExists(publicKey []byte) bool {
	s.mtx.RLock()
	producer := s.getProducer(publicKey)
	s.mtx.RUnlock()
	return producer != nil
}

// ProducerExists returns if a producer is exists by it's owner public key.
func (s *State) ProducerOwnerPublicKeyExists(publicKey []byte) bool {
	s.mtx.RLock()
	key := hex.EncodeToString(publicKey)
	producer := s.getProducerByOwnerPublicKey(key)
	s.mtx.RUnlock()
	return producer != nil
}

// ProducerExists returns if a producer is exists by it's node public key.
func (s *State) ProducerNodePublicKeyExists(publicKey []byte) bool {
	s.mtx.RLock()
	key := hex.EncodeToString(publicKey)
	_, ok := s.NodeOwnerKeys[key]
	s.mtx.RUnlock()
	return ok
}

// SpecialTxExists returns if a special tx (typically means illegal and
// inactive tx) is exists by it's hash
func (s *State) SpecialTxExists(tx *types.Transaction) bool {
	illegalData, ok := tx.Payload.(payload.DPOSIllegalData)
	if !ok {
		log.Error("special tx payload cast failed, tx:", tx.Hash())
		return false
	}

	hash := illegalData.Hash()
	s.mtx.RLock()
	_, ok = s.SpecialTxHashes[hash]
	s.mtx.RUnlock()
	return ok
}

// IsDPOSTransaction returns if a transaction will change the producers and
// votes state.
func (s *State) IsDPOSTransaction(tx *types.Transaction) bool {
	switch tx.TxType {
	// Transactions will changes the producers state.
	case types.RegisterProducer, types.UpdateProducer, types.CancelProducer,
		types.ActivateProducer, types.IllegalProposalEvidence,
		types.IllegalVoteEvidence, types.IllegalBlockEvidence,
		types.IllegalSidechainEvidence, types.InactiveArbitrators,
		types.ReturnDepositCoin:
		return true

	// Transactions will change the producer votes state.
	case types.TransferAsset:
		if tx.Version >= types.TxVersion09 {
			// Votes to producers.
			for _, output := range tx.Outputs {
				if output.Type != types.OTVote {
					continue
				}
				p, _ := output.Payload.(*outputpayload.VoteOutput)
				if p.Version == outputpayload.VoteProducerVersion {
					return true
				} else {
					for _, content := range p.Contents {
						if content.VoteType == outputpayload.Delegate {
							return true
						}
					}
				}
			}
		}
	}

	s.mtx.RLock()
	defer s.mtx.RUnlock()
	// Cancel votes.
	for _, input := range tx.Inputs {
		_, ok := s.Votes[input.ReferKey()]
		if ok {
			return true
		}
	}

	return false
}

// ProcessBlock takes a block and it's confirm to update producers state and
// votes accordingly.
func (s *State) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	s.tryInitProducerAssetAmounts(block.Height)
	s.updateCRInactivePeriod()
	s.processTransactions(block.Transactions, block.Height)
	s.ProcessVoteStatisticsBlock(block)

	if confirm != nil {
		s.countArbitratorsInactivity(block.Height, confirm)
	}

	// Commit changes here if no errors found.
	s.history.Commit(block.Height)
}

// ProcessVoteStatisticsBlock deal with block with vote statistics error.
func (s *State) ProcessVoteStatisticsBlock(block *types.Block) {
	if block.Height == s.chainParams.VoteStatisticsHeight {
		s.processTransactions(block.Transactions, block.Height)
	}
}

// processTransactions takes the transactions and the height when they have been
// packed into a block.  Then loop through the transactions to update producers
// state and votes according to transactions content.
func (s *State) processTransactions(txs []*types.Transaction, height uint32) {

	for _, tx := range txs {
		s.processTransaction(tx, height)
	}

	// Check if any pending producers has got 6 confirms, set them to activate.
	activateProducerFromPending := func(key string, producer *Producer) {
		s.history.Append(height, func() {
			producer.state = Active
			s.ActivityProducers[key] = producer
			delete(s.PendingProducers, key)
		}, func() {
			producer.state = Pending
			s.PendingProducers[key] = producer
			delete(s.ActivityProducers, key)
		})
	}

	// Check if any pending inactive producers has got 6 confirms,
	// then set them to activate.
	activateProducerFromInactive := func(key string, producer *Producer) {
		s.history.Append(height, func() {
			producer.state = Active
			s.ActivityProducers[key] = producer
			delete(s.InactiveProducers, key)
			s.updateCRMemberState(producer.NodePublicKey(), state.MemberElected)
		}, func() {
			producer.state = Inactive
			s.InactiveProducers[key] = producer
			delete(s.ActivityProducers, key)
			s.updateCRMemberState(producer.NodePublicKey(), state.MemberInactive)
		})
	}

	// Check if any pending illegal producers has got 6 confirms,
	// then set them to activate.
	activateProducerFromIllegal := func(key string, producer *Producer) {
		s.history.Append(height, func() {
			producer.state = Active
			s.ActivityProducers[key] = producer
			delete(s.IllegalProducers, key)
			s.updateCRMemberState(producer.NodePublicKey(), state.MemberElected)
		}, func() {
			producer.state = Illegal
			s.IllegalProducers[key] = producer
			delete(s.ActivityProducers, key)
		})
	}

	if len(s.PendingProducers) > 0 {
		for key, producer := range s.PendingProducers {
			if height-producer.registerHeight+1 >= ActivateDuration {
				activateProducerFromPending(key, producer)
			}
		}
	}
	if len(s.InactiveProducers) > 0 {
		for key, producer := range s.InactiveProducers {
			if height > producer.activateRequestHeight &&
				height-producer.activateRequestHeight+1 >= ActivateDuration {
				activateProducerFromInactive(key, producer)
			}
		}
	}
	if height >= s.chainParams.EnableActivateIllegalHeight &&
		len(s.IllegalProducers) > 0 {
		for key, producer := range s.IllegalProducers {
			if height > producer.activateRequestHeight &&
				height-producer.activateRequestHeight+1 >= ActivateDuration {
				activateProducerFromIllegal(key, producer)
			}
		}
	}
}

// processTransaction take a transaction and the height it has been packed into
// a block, then update producers state and votes according to the transaction
// content.
func (s *State) processTransaction(tx *types.Transaction, height uint32) {
	switch tx.TxType {
	case types.RegisterProducer:
		s.registerProducer(tx, height)

	case types.UpdateProducer:
		s.updateProducer(tx.Payload.(*payload.ProducerInfo), height)

	case types.CancelProducer:
		s.cancelProducer(tx.Payload.(*payload.ProcessProducer), height)

	case types.ActivateProducer:
		s.activateProducer(tx.Payload.(*payload.ActivateProducer), height)

	case types.TransferAsset:
		s.processVotes(tx, height)
		s.processDeposit(tx, height)

	case types.IllegalProposalEvidence, types.IllegalVoteEvidence,
		types.IllegalBlockEvidence, types.IllegalSidechainEvidence:
		s.processIllegalEvidence(tx.Payload, height)
		s.recordSpecialTx(tx, height)

	case types.InactiveArbitrators:
		s.processEmergencyInactiveArbitrators(
			tx.Payload.(*payload.InactiveArbitrators), height)
		s.recordSpecialTx(tx, height)

	case types.ReturnDepositCoin:
		s.returnDeposit(tx, height)
		s.processDeposit(tx, height)

	case types.UpdateVersion:
		s.updateVersion(tx, height)
	}

	s.processCancelVotes(tx, height)
}

// registerProducer handles the register producer transaction.
func (s *State) registerProducer(tx *types.Transaction, height uint32) {
	info := tx.Payload.(*payload.ProducerInfo)
	nickname := info.NickName
	nodeKey := hex.EncodeToString(info.NodePublicKey)
	ownerKey := hex.EncodeToString(info.OwnerPublicKey)
	// ignore error here because this converting process has been ensured in
	// the context check already
	programHash, _ := contract.PublicKeyToDepositProgramHash(info.
		OwnerPublicKey)

	amount := common.Fixed64(0)
	depositOutputs := make(map[string]common.Fixed64)
	for i, output := range tx.Outputs {
		if output.ProgramHash.IsEqual(*programHash) {
			amount += output.Value
			op := types.NewOutPoint(tx.Hash(), uint16(i))
			depositOutputs[op.ReferKey()] = output.Value
		}
	}

	producer := Producer{
		info:                   *info,
		registerHeight:         height,
		votes:                  0,
		inactiveSince:          0,
		inactiveCountingHeight: 0,
		penalty:                common.Fixed64(0),
		activateRequestHeight:  math.MaxUint32,
		depositAmount:          amount,
		depositHash:            *programHash,
	}

	s.history.Append(height, func() {
		s.Nicknames[nickname] = struct{}{}
		s.NodeOwnerKeys[nodeKey] = ownerKey
		s.PendingProducers[ownerKey] = &producer
		s.ProducerDepositMap[*programHash] = struct{}{}
		for k, v := range depositOutputs {
			s.DepositOutputs[k] = v
		}
	}, func() {
		delete(s.Nicknames, nickname)
		delete(s.NodeOwnerKeys, nodeKey)
		delete(s.PendingProducers, ownerKey)
		delete(s.ProducerDepositMap, *programHash)
		for k := range depositOutputs {
			delete(s.DepositOutputs, k)
		}
	})
}

// updateProducer handles the update producer transaction.
func (s *State) updateProducer(info *payload.ProducerInfo, height uint32) {
	producer := s.getProducer(info.OwnerPublicKey)
	producerInfo := producer.info
	s.history.Append(height, func() {
		s.updateProducerInfo(&producerInfo, info)
	}, func() {
		s.updateProducerInfo(info, &producerInfo)
	})
}

// cancelProducer handles the cancel producer transaction.
func (s *State) cancelProducer(payload *payload.ProcessProducer, height uint32) {
	key := hex.EncodeToString(payload.OwnerPublicKey)
	producer := s.getProducer(payload.OwnerPublicKey)
	isPending := producer.state == Pending
	s.history.Append(height, func() {
		producer.state = Canceled
		producer.cancelHeight = height
		s.CanceledProducers[key] = producer
		if isPending {
			delete(s.PendingProducers, key)
			s.PendingCanceledProducers[key] = producer
		} else {
			delete(s.ActivityProducers, key)
		}
		delete(s.Nicknames, producer.info.NickName)
	}, func() {
		producer.cancelHeight = 0
		delete(s.CanceledProducers, key)
		if isPending {
			producer.state = Pending
			s.PendingProducers[key] = producer
			delete(s.PendingCanceledProducers, key)
		} else {
			producer.state = Active
			s.ActivityProducers[key] = producer
			s.updateCRMemberState(producer.NodePublicKey(), state.MemberElected)
		}
		s.Nicknames[producer.info.NickName] = struct{}{}
	})
}

// activateProducer handles the activate producer transaction.
func (s *State) activateProducer(p *payload.ActivateProducer, height uint32) {
	producer := s.getProducer(p.NodePublicKey)
	if producer == nil {
		log.Error("can't find producer to activate")
		return
	}
	s.history.Append(height, func() {
		producer.activateRequestHeight = height
	}, func() {
		producer.activateRequestHeight = math.MaxUint32
	})
}

// processVotes takes a transaction, if the transaction including any vote
// inputs or outputs, validate and update producers votes.
func (s *State) processVotes(tx *types.Transaction, height uint32) {
	if tx.Version >= types.TxVersion09 {
		// Votes to producers.
		for i, output := range tx.Outputs {
			if output.Type != types.OTVote {
				continue
			}
			p, _ := output.Payload.(*outputpayload.VoteOutput)
			if p.Version == outputpayload.VoteProducerVersion {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				s.history.Append(height, func() {
					s.Votes[op.ReferKey()] = struct{}{}
				}, func() {
					delete(s.Votes, op.ReferKey())
				})
				s.processVoteOutput(output, height)
			} else {
				var exist bool
				for _, content := range p.Contents {
					if content.VoteType == outputpayload.Delegate {
						exist = true
						break
					}
				}
				if exist {
					op := types.NewOutPoint(tx.Hash(), uint16(i))
					s.history.Append(height, func() {
						s.Votes[op.ReferKey()] = struct{}{}
					}, func() {
						delete(s.Votes, op.ReferKey())
					})
					s.processVoteOutput(output, height)
				}
			}
		}
	}
}

// tryInitProducerAssetAmounts will initialize deposit amount of all
// producers after CR voting start height.
func (s *State) tryInitProducerAssetAmounts(blockHeight uint32) {
	if blockHeight != s.chainParams.CRVotingStartHeight {
		return
	}

	setAmount := func(producer *Producer, amount common.Fixed64) {
		s.history.Append(blockHeight, func() {
			producer.depositAmount = amount
		}, func() {
			producer.depositAmount = common.Fixed64(0)
		})
	}

	producers := s.getAllProducers()
	for _, v := range producers {
		programHash, err := contract.PublicKeyToDepositProgramHash(
			v.info.OwnerPublicKey)
		if err != nil {
			log.Warn(err)
			continue
		}

		amount, err := s.getProducerDepositAmount(*programHash)
		if err != nil {
			log.Warn(err)
			continue
		}

		producer := v
		setAmount(producer, amount)
	}
}

// processDeposit takes a transaction output with deposit program hash.
func (s *State) processDeposit(tx *types.Transaction, height uint32) {
	for i, output := range tx.Outputs {
		if contract.GetPrefixType(output.ProgramHash) ==
			contract.PrefixDeposit {
			if s.addProducerAssert(output, height) {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				s.DepositOutputs[op.ReferKey()] = output.Value
			}
		}
	}
}

// getProducerByDepositHash will try to get producer with specified program
// hash, note the producer state should be pending active or inactive.
func (s *State) getProducerByDepositHash(hash common.Uint168) *Producer {
	for _, producer := range s.PendingProducers {
		if producer.depositHash.IsEqual(hash) {
			return producer
		}
	}
	for _, producer := range s.ActivityProducers {
		if producer.depositHash.IsEqual(hash) {
			return producer
		}
	}
	for _, producer := range s.InactiveProducers {
		if producer.depositHash.IsEqual(hash) {
			return producer
		}
	}
	for _, producer := range s.CanceledProducers {
		if producer.depositHash.IsEqual(hash) {
			return producer
		}
	}
	for _, producer := range s.IllegalProducers {
		if producer.depositHash.IsEqual(hash) {
			return producer
		}
	}
	return nil
}

// addProducerAssert will plus deposit amount for producers referenced in
// program hash of transaction output.
func (s *State) addProducerAssert(output *types.Output, height uint32) bool {
	if producer := s.getProducerByDepositHash(output.ProgramHash); producer != nil {
		s.history.Append(height, func() {
			producer.depositAmount += output.Value
		}, func() {
			producer.depositAmount -= output.Value
		})
		return true
	}
	return false
}

// processCancelVotes takes a transaction output with vote payload.
func (s *State) processCancelVotes(tx *types.Transaction, height uint32) {
	var exist bool
	for _, input := range tx.Inputs {
		referKey := input.ReferKey()
		if _, ok := s.Votes[referKey]; ok {
			exist = true
		}
	}
	if !exist {
		return
	}

	references, err := s.getTxReference(tx)
	if err != nil {
		log.Errorf("get tx reference failed, tx hash:%s", tx.Hash())
		return
	}
	for _, input := range tx.Inputs {
		referKey := input.ReferKey()
		_, ok := s.Votes[referKey]
		if ok {
			out := references[input]
			s.processVoteCancel(&out, height)
		}
	}
}

// processVoteOutput takes a transaction output with vote payload.
func (s *State) processVoteOutput(output *types.Output, height uint32) {
	countByGross := func(producer *Producer) {
		s.history.Append(height, func() {
			producer.votes += output.Value
		}, func() {
			producer.votes -= output.Value
		})
	}

	countByVote := func(producer *Producer, vote common.Fixed64) {
		s.history.Append(height, func() {
			producer.votes += vote
		}, func() {
			producer.votes -= vote
		})
	}

	p := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range p.Contents {
		for _, cv := range vote.CandidateVotes {
			producer := s.getProducer(cv.Candidate)
			if producer == nil {
				continue
			}

			switch vote.VoteType {
			case outputpayload.Delegate:
				if p.Version == outputpayload.VoteProducerVersion {
					countByGross(producer)
				} else {
					v := cv.Votes
					countByVote(producer, v)
				}
			}
		}
	}
}

// processVoteCancel takes a previous vote output and decrease producers votes.
func (s *State) processVoteCancel(output *types.Output, height uint32) {
	subtractByGross := func(producer *Producer) {
		s.history.Append(height, func() {
			producer.votes -= output.Value
		}, func() {
			producer.votes += output.Value
		})
	}

	subtractByVote := func(producer *Producer, vote common.Fixed64) {
		s.history.Append(height, func() {
			producer.votes -= vote
		}, func() {
			producer.votes += vote
		})
	}

	p := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range p.Contents {
		for _, cv := range vote.CandidateVotes {
			producer := s.getProducer(cv.Candidate)
			if producer == nil {
				continue
			}
			switch vote.VoteType {
			case outputpayload.Delegate:
				if p.Version == outputpayload.VoteProducerVersion {
					subtractByGross(producer)
				} else {
					v := cv.Votes
					subtractByVote(producer, v)
				}
			}
		}
	}
}

// returnDeposit change producer state to ReturnedDeposit
func (s *State) returnDeposit(tx *types.Transaction, height uint32) {
	var inputValue common.Fixed64
	for _, input := range tx.Inputs {
		inputValue += s.DepositOutputs[input.ReferKey()]
	}

	returnAction := func(producer *Producer) {
		s.history.Append(height, func() {
			if height >= s.chainParams.CRVotingStartHeight {
				producer.depositAmount -= inputValue
			}
			producer.state = Returned
		}, func() {
			if height >= s.chainParams.CRVotingStartHeight {
				producer.depositAmount += inputValue
			}
			producer.state = Canceled
		})
	}

	for _, program := range tx.Programs {
		pk := program.Code[1 : len(program.Code)-1]
		if producer := s.getProducer(pk); producer != nil && producer.state == Canceled {
			returnAction(producer)
		}
	}
}

// updateVersion record the update period during that inactive arbitrators
// will not need to pay the penalty
func (s *State) updateVersion(tx *types.Transaction, height uint32) {
	p, ok := tx.Payload.(*payload.UpdateVersion)
	if !ok {
		log.Error("tx payload cast failed, tx:", tx.Hash())
		return
	}

	oriVersionStartHeight := s.VersionStartHeight
	oriVersionEndHeight := s.VersionEndHeight
	s.history.Append(height, func() {
		s.VersionStartHeight = p.StartHeight
		s.VersionEndHeight = p.EndHeight
	}, func() {
		s.VersionStartHeight = oriVersionStartHeight
		s.VersionEndHeight = oriVersionEndHeight
	})
}

// processEmergencyInactiveArbitrators change producer state according to
// emergency inactive arbitrators
func (s *State) processEmergencyInactiveArbitrators(
	inactivePayload *payload.InactiveArbitrators, height uint32) {

	addEmergencyInactiveArbitrator := func(key string, producer *Producer) {
		s.history.Append(height, func() {
			s.setInactiveProducer(producer, key, height, true)
			s.EmergencyInactiveArbiters[key] = struct{}{}
		}, func() {
			s.revertSettingInactiveProducer(producer, key, height, true)
			delete(s.EmergencyInactiveArbiters, key)
		})
	}

	for _, v := range inactivePayload.Arbitrators {
		nodeKey := hex.EncodeToString(v)
		key, ok := s.NodeOwnerKeys[nodeKey]
		if !ok {
			continue
		}

		if p, ok := s.ActivityProducers[key]; ok {
			addEmergencyInactiveArbitrator(key, p)
		}
		if p, ok := s.InactiveProducers[key]; ok {
			addEmergencyInactiveArbitrator(key, p)
		}
	}
}

// recordSpecialTx record hash of a special tx
func (s *State) recordSpecialTx(tx *types.Transaction, height uint32) {
	illegalData, ok := tx.Payload.(payload.DPOSIllegalData)
	if !ok {
		log.Error("special tx payload cast failed, tx:", tx.Hash())
		return
	}

	hash := illegalData.Hash()
	s.history.Append(height, func() {
		s.SpecialTxHashes[hash] = struct{}{}
	}, func() {
		delete(s.SpecialTxHashes, hash)
	})
}

// processIllegalEvidence takes the illegal evidence payload and change producer
// state according to the evidence.
func (s *State) processIllegalEvidence(payloadData types.Payload,
	height uint32) {
	// Get illegal producers from evidence.
	var illegalProducers [][]byte
	switch p := payloadData.(type) {
	case *payload.DPOSIllegalProposals:
		illegalProducers = [][]byte{p.Evidence.Proposal.Sponsor}

	case *payload.DPOSIllegalVotes:
		illegalProducers = [][]byte{p.Evidence.Vote.Signer}

	case *payload.DPOSIllegalBlocks:
		signers := make(map[string]interface{})
		for _, pk := range p.Evidence.Signers {
			signers[hex.EncodeToString(pk)] = nil
		}

		for _, pk := range p.CompareEvidence.Signers {
			key := hex.EncodeToString(pk)
			if _, ok := signers[key]; ok {
				illegalProducers = append(illegalProducers, pk)
			}
		}

	case *payload.SidechainIllegalData:
		illegalProducers = [][]byte{p.IllegalSigner}

	default:
		return
	}

	// Set illegal producers to FoundBad state
	for _, pk := range illegalProducers {
		key, ok := s.NodeOwnerKeys[hex.EncodeToString(pk)]
		if !ok {
			continue
		}
		if producer, ok := s.ActivityProducers[key]; ok {
			s.history.Append(height, func() {
				producer.state = Illegal
				producer.illegalHeight = height
				s.IllegalProducers[key] = producer
				producer.activateRequestHeight = math.MaxUint32
				delete(s.ActivityProducers, key)
				delete(s.Nicknames, producer.info.NickName)
			}, func() {
				producer.state = Active
				producer.illegalHeight = 0
				s.ActivityProducers[key] = producer
				producer.activateRequestHeight = math.MaxUint32
				delete(s.IllegalProducers, key)
				s.Nicknames[producer.info.NickName] = struct{}{}
				s.updateCRMemberState(producer.NodePublicKey(), state.MemberElected)
			})
			continue
		}

		if producer, ok := s.CanceledProducers[key]; ok {
			s.history.Append(height, func() {
				producer.state = Illegal
				producer.illegalHeight = height
				s.IllegalProducers[key] = producer
				delete(s.CanceledProducers, key)
				delete(s.Nicknames, producer.info.NickName)
			}, func() {
				producer.state = Canceled
				producer.illegalHeight = 0
				s.CanceledProducers[key] = producer
				delete(s.IllegalProducers, key)
				s.Nicknames[producer.info.NickName] = struct{}{}
			})
			continue
		}
	}
}

// ProcessIllegalBlockEvidence takes a illegal block payload and change the
// producers state immediately.  This is a spacial case that can be handled
// before it packed into a block.
func (s *State) ProcessSpecialTxPayload(p types.Payload, height uint32) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	if inactivePayload, ok := p.(*payload.InactiveArbitrators); ok {
		s.processEmergencyInactiveArbitrators(inactivePayload, 0)
	} else {
		s.processIllegalEvidence(p, 0)
	}

	// Commit changes here if no errors found.
	s.history.Commit(height)
}

func (s *State) updateCRMemberState(nodePublicKey []byte, memberState state.MemberState) {
	if s.getCRMembers != nil {
		crMembers := s.getCRMembers()
		for _, cr := range crMembers {
			if bytes.Equal(cr.DPOSPublicKey, nodePublicKey) {
				cr.MemberState = memberState
			}
		}
	}
}

func (s *State) updateCRInactivePeriod() {
	if s.getCRMembers != nil {
		crMembers := s.getCRMembers()
		for _, cr := range crMembers {
			if cr.MemberState == state.MemberInactive {
				cr.InactiveCount += 1
			}
		}
	}
}

// setInactiveProducer set active producer to inactive state
func (s *State) setInactiveProducer(producer *Producer, key string,
	height uint32, emergency bool) {
	producer.inactiveSince = height
	producer.activateRequestHeight = math.MaxUint32
	producer.state = Inactive
	s.InactiveProducers[key] = producer
	delete(s.ActivityProducers, key)

	s.updateCRMemberState(producer.NodePublicKey(), state.MemberInactive)

	if height < s.VersionStartHeight || height >= s.VersionEndHeight {
		if !emergency {
			producer.penalty += s.chainParams.InactivePenalty
		} else {
			producer.penalty += s.chainParams.EmergencyInactivePenalty
		}
	}
}

// revertSettingInactiveProducer revert operation about setInactiveProducer
func (s *State) revertSettingInactiveProducer(producer *Producer, key string,
	height uint32, emergency bool) {
	producer.inactiveSince = 0
	producer.activateRequestHeight = math.MaxUint32
	producer.state = Active
	s.ActivityProducers[key] = producer
	delete(s.InactiveProducers, key)
	s.updateCRMemberState(producer.NodePublicKey(), state.MemberElected)

	if height < s.VersionStartHeight || height >= s.VersionEndHeight {
		penalty := s.chainParams.InactivePenalty
		if emergency {
			penalty = s.chainParams.EmergencyInactivePenalty
		}

		if producer.penalty < penalty {
			producer.penalty = common.Fixed64(0)
		} else {
			producer.penalty -= penalty
		}
	}
}

// countArbitratorsInactivity count arbitrators inactive rounds, and change to
// inactive if more than "MaxInactiveRounds"
func (s *State) countArbitratorsInactivity(height uint32,
	confirm *payload.Confirm) {
	// check inactive arbitrators after producers has participated in
	if height < s.chainParams.PublicDPOSHeight {
		return
	}

	// changingArbiters indicates the arbiters that should reset inactive
	// counting state. With the value of true means the producer is on duty or
	// is not current arbiter any more, or just becoming current arbiter; and
	// false means producer is arbiter in both heights and not on duty.
	changingArbiters := make(map[string]bool)
	for k := range s.PreBlockArbiters {
		changingArbiters[k] = true
	}
	s.PreBlockArbiters = make(map[string]struct{})
	for _, a := range s.getArbiters() {
		key := s.getProducerKey(a)
		s.PreBlockArbiters[key] = struct{}{}
		if _, exist := changingArbiters[key]; exist {
			changingArbiters[key] = false
		}
	}
	changingArbiters[s.getProducerKey(confirm.Proposal.Sponsor)] = true

	// CRC producers are not in the ActivityProducers,
	// so they will not be inactive
	for k, v := range changingArbiters {
		key := k // avoiding pass iterator to closure
		producer, ok := s.ActivityProducers[key]
		if !ok {
			continue
		}
		countingHeight := producer.inactiveCountingHeight
		needReset := v // avoiding pass iterator to closure

		s.history.Append(height, func() {
			s.tryUpdateInactivity(key, producer, needReset, height)
		}, func() {
			s.tryRevertInactivity(key, producer, needReset, height, countingHeight)
		})
	}
}

func (s *State) tryRevertInactivity(key string, producer *Producer,
	needReset bool, height, startHeight uint32) {
	if needReset {
		producer.inactiveCountingHeight = startHeight
		return
	}

	if producer.inactiveCountingHeight == height {
		producer.inactiveCountingHeight = 0
	}

	if producer.state == Inactive {
		s.revertSettingInactiveProducer(producer, key, height, false)
		producer.inactiveCountingHeight = startHeight
	}
}

func (s *State) tryUpdateInactivity(key string, producer *Producer,
	needReset bool, height uint32) {
	if needReset {
		producer.inactiveCountingHeight = 0
		return
	}

	if producer.inactiveCountingHeight == 0 {
		producer.inactiveCountingHeight = height
	}

	if height-producer.inactiveCountingHeight >= s.chainParams.MaxInactiveRounds {
		s.setInactiveProducer(producer, key, height, false)
		producer.inactiveCountingHeight = 0
	}
}

// RollbackTo restores the database state to the given height, if no enough
// history to rollback to return error.
func (s *State) RollbackTo(height uint32) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()
	return s.history.RollbackTo(height)
}

// GetHistory returns a history state instance storing the producers and votes
// on the historical height.
func (s *State) GetHistory(height uint32) (*StateKeyFrame, error) {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	// Seek to state to target height.
	if err := s.history.SeekTo(height); err != nil {
		return nil, err
	}

	// Take a snapshot of the history.
	return s.snapshot(), nil
}

// NewState returns a new State instance.
func NewState(chainParams *config.Params, getArbiters func() [][]byte, getCRMembers func() []*state.CRMember,
	getProducerDepositAmount func(common.Uint168) (common.Fixed64, error)) *State {
	return &State{
		chainParams:              chainParams,
		getArbiters:              getArbiters,
		getCRMembers:             getCRMembers,
		getProducerDepositAmount: getProducerDepositAmount,
		history:                  utils.NewHistory(maxHistoryCapacity),
		StateKeyFrame:            NewStateKeyFrame(),
	}
}
