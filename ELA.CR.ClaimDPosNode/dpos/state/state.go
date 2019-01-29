package state

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"math"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

// ProducerState represents the state of a producer.
type ProducerState byte

const (
	// Pending indicates the producer is just registered and didn't get 6
	// confirmations yet.
	Pending ProducerState = iota

	// Activate indicates the producer is registered and confirmed by more than
	// 6 blocks.
	Activate

	// Inactivate indicates the producer has been inactive for a period which shall
	// be punished and will be activate later
	Inactivate

	// Canceled indicates the producer was canceled.
	Canceled

	// FoundBad indicates the producer was found doing bad.
	FoundBad
)

// producerStateStrings is a array of producer states back to their constant
// names for pretty printing.
var producerStateStrings = []string{"Pending", "Activate", "Inactivate",
	"Canceled", "FoundBad"}

func (ps ProducerState) String() string {
	if int(ps) < len(producerStateStrings) {
		return producerStateStrings[ps]
	}
	return fmt.Sprintf("ProducerState-%d", ps)
}

// Producer holds a producer's info.  It provides read only methods to access
// producer's info.
type Producer struct {
	info           payload.ProducerInfo
	state          ProducerState
	registerHeight uint32
	cancelHeight   uint32
	inactiveRounds uint32
	inactiveSince  uint32
	penalty        common.Fixed64
	votes          common.Fixed64
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

func (p *Producer) Penalty() common.Fixed64 {
	return p.penalty
}

const (
	// maxHistoryCapacity indicates the maximum capacity of change history.
	maxHistoryCapacity = 10

	// snapshotInterval is the time interval to take a snapshot of the state.
	snapshotInterval = 12

	// maxSnapshots is the maximum newest snapshots keeps in memory.
	maxSnapshots = 9
)

// State is a memory database storing DPOS producers state, like pending
// producers active producers and their votes.
type State struct {
	arbiters    interfaces.Arbitrators
	chainParams *config.Params

	mtx               sync.RWMutex
	nodeOwnerKeys     map[string]string // NodePublicKey as key, OwnerPublicKey as value
	pendingProducers  map[string]*Producer
	activityProducers map[string]*Producer
	inactiveProducers map[string]*Producer
	canceledProducers map[string]*Producer
	illegalProducers  map[string]*Producer
	votes             map[string]*types.Output
	nicknames         map[string]struct{}
	history           *history

	// snapshots is the data set of DPOS state snapshots, it takes a snapshot of
	// state every 12 blocks, and keeps at most 9 newest snapshots in memory.
	snapshots [maxSnapshots]*State
	cursor    int
}

// getProducerKey returns the producer's owner public key string, whether the
// given public key is the producer's node public key or owner public key.
func (s *State) getProducerKey(publicKey []byte) string {
	key := hex.EncodeToString(publicKey)

	// If the given public key is node public key, get the producer's owner
	// public key.
	if owner, ok := s.nodeOwnerKeys[key]; ok {
		return owner
	}

	return key
}

// getProducer returns a producer with the producer's node public key or it's
// owner public key, if no matches return nil.
func (s *State) getProducer(publicKey []byte) *Producer {
	key := s.getProducerKey(publicKey)
	if producer, ok := s.activityProducers[key]; ok {
		return producer
	}
	if producer, ok := s.canceledProducers[key]; ok {
		return producer
	}
	if producer, ok := s.illegalProducers[key]; ok {
		return producer
	}
	if producer, ok := s.pendingProducers[key]; ok {
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
		delete(s.nicknames, origin.NickName)
		s.nicknames[update.NickName] = struct{}{}
	}

	// compare and update node public key, we only query pending and active node
	// because canceled and illegal node can not be updated.
	if !bytes.Equal(origin.NodePublicKey, update.NodePublicKey) {
		oldKey := hex.EncodeToString(origin.NodePublicKey)
		newKey := hex.EncodeToString(update.NodePublicKey)
		delete(s.nodeOwnerKeys, oldKey)
		s.nodeOwnerKeys[newKey] = hex.EncodeToString(origin.OwnerPublicKey)
	}

	producer.info = *update
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
	producers := make([]*Producer, 0, len(s.pendingProducers)+
		len(s.activityProducers))
	for _, producer := range s.pendingProducers {
		producers = append(producers, producer)
	}
	for _, producer := range s.activityProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetPendingProducers returns all producers that in pending state.
func (s *State) GetPendingProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.pendingProducers))
	for _, producer := range s.pendingProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetActiveProducers returns all producers that in active state.
func (s *State) GetActiveProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.activityProducers))
	for _, producer := range s.activityProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetCanceledProducers returns all producers that in cancel state.
func (s *State) GetCanceledProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.canceledProducers))
	for _, producer := range s.canceledProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// GetIllegalProducers returns all illegal producers.
func (s *State) GetIllegalProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.illegalProducers))
	for _, producer := range s.illegalProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

func (s *State) GetInactiveProducers() []*Producer {
	s.mtx.RLock()
	producers := make([]*Producer, 0, len(s.inactiveProducers))
	for _, producer := range s.inactiveProducers {
		producers = append(producers, producer)
	}
	s.mtx.RUnlock()
	return producers
}

// IsPendingProducer returns if a producer is in pending list according to the
// public key.
func (s *State) IsPendingProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.pendingProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsActiveProducer returns if a producer is in activate list according to the
// public key.
func (s *State) IsActiveProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.activityProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsInactiveProducer returns if a producer is in inactivate list according to
// the public key.
func (s *State) IsInactiveProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.inactiveProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsCanceledProducer returns if a producer is in canceled list according to the
// public key.
func (s *State) IsCanceledProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.canceledProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsIllegalProducer returns if a producer is in illegal list according to the
// public key.
func (s *State) IsIllegalProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.illegalProducers[s.getProducerKey(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// NicknameExists returns if a nickname is exists.
func (s *State) NicknameExists(nickname string) bool {
	s.mtx.RLock()
	_, ok := s.nicknames[nickname]
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

// IsDPOSTransaction returns if a transaction will change the producers and
// votes state.
func (s *State) IsDPOSTransaction(tx *types.Transaction) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	switch tx.TxType {
	// Transactions will changes the producers state.
	case types.RegisterProducer, types.UpdateProducer, types.CancelProducer,
		types.IllegalProposalEvidence, types.IllegalVoteEvidence,
		types.IllegalBlockEvidence, types.IllegalSidechainEvidence:
		return true

	// Transactions will change the votes state.
	case types.TransferAsset:
		if tx.Version >= types.TxVersion09 {
			// Votes to producers.
			for _, output := range tx.Outputs {
				if output.Type == types.OTVote {
					return true
				}
			}
		}

		// Cancel votes.
		for _, input := range tx.Inputs {
			_, ok := s.votes[input.ReferKey()]
			if ok {
				return true
			}
		}
	}

	return false
}

// ProcessBlock takes a block and it's confirm to update producers state and
// votes accordingly.
func (s *State) ProcessBlock(block *types.Block, confirm *types.DPosProposalVoteSlot) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	s.processTransactions(block.Transactions, block.Height)

	if confirm != nil {
		arbiters := s.getInactiveArbitrators(confirm)
		s.countArbitratorsInactivity(block.Height, arbiters)
		s.tryLeaveInactiveMode(block.Height)
	}

	// Take snapshot when snapshot point arrives.
	if (block.Height-s.chainParams.DPOSStartHeight)%snapshotInterval == 0 {
		s.cursor = s.cursor % maxSnapshots
		s.snapshots[s.cursor] = s.snapshot()
		s.cursor++
	}

	// Commit changes here if no errors found.
	s.history.commit(block.Height)
}

// getInactiveArbitrators returns inactive arbiters from a confirm data.
func (s *State) getInactiveArbitrators(confirm *types.DPosProposalVoteSlot) (result []string) {
	if bytes.Equal(s.arbiters.GetOnDutyArbitrator(), confirm.Proposal.Sponsor) {

		arSequence := s.arbiters.GetArbitrators()
		arSequence = append(arSequence, arSequence...)

		start := s.arbiters.GetOnDutyArbitrator()
		stop := confirm.Proposal.Sponsor
		reachedStart := false

		for i := 0; i < len(arSequence)-1; i++ {
			if bytes.Equal(start, arSequence[i]) {
				reachedStart = true
			}

			if reachedStart {
				if bytes.Equal(stop, arSequence[i]) {
					break
				}

				result = append(result, hex.EncodeToString(arSequence[i]))
			}
		}
	}

	return result
}

// processTransactions takes the transactions and the height when they have been
// packed into a block.  Then loop through the transactions to update producers
// state and votes according to transactions content.
func (s *State) processTransactions(txs []*types.Transaction, height uint32) {
	for _, tx := range txs {
		s.processTransaction(tx, height)
	}

	// Check if any pending producers has got 6 confirms, set them to activate.
	activeProducer := func(key string, producer *Producer) {
		s.history.append(height, func() {
			producer.state = Activate
			s.activityProducers[key] = producer
			delete(s.pendingProducers, key)
		}, func() {
			producer.state = Pending
			s.pendingProducers[key] = producer
			delete(s.activityProducers, key)
		})
	}
	if len(s.pendingProducers) > 0 {
		for key, producer := range s.pendingProducers {
			if height-producer.registerHeight >= 6 {
				activeProducer(key, producer)
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
		s.registerProducer(tx.Payload.(*payload.ProducerInfo),
			height)

	case types.UpdateProducer:
		s.updateProducer(tx.Payload.(*payload.ProducerInfo),
			height)

	case types.CancelProducer:
		s.cancelProducer(tx.Payload.(*payload.CancelProducer),
			height)

	case types.TransferAsset:
		s.processVotes(tx, height)

	case types.IllegalProposalEvidence, types.IllegalVoteEvidence,
		types.IllegalBlockEvidence, types.IllegalSidechainEvidence:
		s.processIllegalEvidence(tx.Payload, height)

	case types.InactiveArbitrators:
		s.processEmergencyInactiveArbitrators(
			tx.Payload.(*payload.InactiveArbitrators), height)
	}
}

// registerProducer handles the register producer transaction.
func (s *State) registerProducer(payload *payload.ProducerInfo, height uint32) {
	nickname := payload.NickName
	nodeKey := hex.EncodeToString(payload.NodePublicKey)
	ownerKey := hex.EncodeToString(payload.OwnerPublicKey)
	producer := Producer{
		info:           *payload,
		registerHeight: height,
		votes:          0,
		inactiveRounds: 0,
		inactiveSince:  math.MaxUint32,
		penalty:        common.Fixed64(0),
	}

	s.history.append(height, func() {
		s.nicknames[nickname] = struct{}{}
		s.nodeOwnerKeys[nodeKey] = ownerKey
		s.pendingProducers[ownerKey] = &producer
	}, func() {
		delete(s.nicknames, nickname)
		delete(s.nodeOwnerKeys, nodeKey)
		delete(s.pendingProducers, ownerKey)
	})
}

// updateProducer handles the update producer transaction.
func (s *State) updateProducer(info *payload.ProducerInfo, height uint32) {
	producer := s.getProducer(info.OwnerPublicKey)
	producerInfo := producer.info
	s.history.append(height, func() {
		s.updateProducerInfo(&producerInfo, info)
	}, func() {
		s.updateProducerInfo(info, &producerInfo)
	})
}

// cancelProducer handles the cancel producer transaction.
func (s *State) cancelProducer(payload *payload.CancelProducer, height uint32) {
	key := hex.EncodeToString(payload.OwnerPublicKey)
	producer := s.getProducer(payload.OwnerPublicKey)
	s.history.append(height, func() {
		producer.state = Canceled
		producer.cancelHeight = height
		s.canceledProducers[key] = producer
		delete(s.activityProducers, key)
		delete(s.nicknames, producer.info.NickName)
	}, func() {
		producer.state = Activate
		producer.cancelHeight = 0
		delete(s.canceledProducers, key)
		s.activityProducers[key] = producer
		s.nicknames[producer.info.NickName] = struct{}{}
	})
}

// processVotes takes a transaction, if the transaction including any vote
// inputs or outputs, validate and update producers votes.
func (s *State) processVotes(tx *types.Transaction, height uint32) {
	if tx.Version >= types.TxVersion09 {
		// Votes to producers.
		for i, output := range tx.Outputs {
			if output.Type == types.OTVote {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				s.votes[op.ReferKey()] = output
				s.processVoteOutput(output, height)
			}
		}
	}

	// Cancel votes.
	for _, input := range tx.Inputs {
		output, ok := s.votes[input.ReferKey()]
		if ok {
			s.processVoteCancel(output, height)
		}
	}
}

// processVoteOutput takes a transaction output with vote payload.
func (s *State) processVoteOutput(output *types.Output, height uint32) {
	payload := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range payload.Contents {
		for _, candidate := range vote.Candidates {
			key := hex.EncodeToString(candidate)
			producer := s.activityProducers[key]
			switch vote.VoteType {
			case outputpayload.CRC:
				// TODO separate CRC and Delegate votes.
				fallthrough
			case outputpayload.Delegate:
				s.history.append(height, func() {
					producer.votes += output.Value
				}, func() {
					producer.votes -= output.Value
				})
			}
		}
	}
}

// processVoteCancel takes a previous vote output and decrease producers votes.
func (s *State) processVoteCancel(output *types.Output, height uint32) {
	payload := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range payload.Contents {
		for _, candidate := range vote.Candidates {
			producer := s.getProducer(candidate)
			if producer == nil {
				// This should not happen, just in case.
				continue
			}
			switch vote.VoteType {
			case outputpayload.CRC:
				// TODO separate CRC and Delegate votes.
				fallthrough
			case outputpayload.Delegate:
				s.history.append(height, func() {
					producer.votes -= output.Value
				}, func() {
					producer.votes += output.Value
				})
			}
		}
	}
}

// processEmergencyInactiveArbitrators change producer state according to
// emergency inactive arbitrators
func (s *State) processEmergencyInactiveArbitrators(
	inactivePayload *payload.InactiveArbitrators, height uint32) {

	addEmergencyInactiveArbitrator := func(key string, producer *Producer) {
		s.history.append(height, func() {

			producer.state = Inactivate
			s.inactiveProducers[key] = producer
			delete(s.activityProducers, key)

			producer.penalty += s.chainParams.EmergencyInactivePenalty
		}, func() {

			producer.state = Activate
			s.activityProducers[key] = producer
			delete(s.inactiveProducers, key)

			if producer.penalty < s.chainParams.EmergencyInactivePenalty {
				producer.penalty = common.Fixed64(0)
			} else {
				producer.penalty -= s.chainParams.EmergencyInactivePenalty
			}
		})
	}

	for _, v := range inactivePayload.Arbitrators {
		pkStr := common.BytesToHexString(v)

		if _, ok := s.activityProducers[pkStr]; ok {
			addEmergencyInactiveArbitrator(pkStr, s.activityProducers[pkStr])
		} else {
			if s.arbiters.IsCRCArbitrator(v) {
				// add temporary producer obj for crc inactive arbitrator
				producer := &Producer{
					info: payload.ProducerInfo{
						NodePublicKey: v,
					},
					registerHeight: height,
					votes:          0,
					inactiveRounds: 0,
					inactiveSince:  math.MaxUint32,
					penalty:        common.Fixed64(0),
				}
				addEmergencyInactiveArbitrator(pkStr, producer)

			} else {
				log.Warn("unknown active producer: ", v)
			}
		}
	}

}

// processIllegalEvidence takes the illegal evidence payload and change producer
// state according to the evidence.
func (s *State) processIllegalEvidence(payload types.Payload, height uint32) {
	// Get illegal producers from evidence.
	var illegalProducers [][]byte
	switch p := payload.(type) {
	case *types.PayloadIllegalProposal:
		illegalProducers = [][]byte{p.Evidence.Proposal.Sponsor}

	case *types.PayloadIllegalVote:
		illegalProducers = [][]byte{p.Evidence.Vote.Signer}

	case *types.PayloadIllegalBlock:
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

	case *types.PayloadSidechainIllegalData:
		illegalProducers = [][]byte{p.IllegalSigner}

	default:
		return
	}

	// Set illegal producers to FoundBad state
	for _, pk := range illegalProducers {
		key := hex.EncodeToString(pk)
		if producer, ok := s.activityProducers[key]; ok {
			s.history.append(height, func() {
				producer.state = FoundBad
				s.illegalProducers[key] = producer
				delete(s.activityProducers, key)
				delete(s.nicknames, producer.info.NickName)
			}, func() {
				producer.state = Activate
				s.activityProducers[key] = producer
				delete(s.illegalProducers, key)
				s.nicknames[producer.info.NickName] = struct{}{}
			})
			continue
		}

		if producer, ok := s.canceledProducers[key]; ok {
			s.history.append(height, func() {
				producer.state = FoundBad
				s.illegalProducers[key] = producer
				delete(s.canceledProducers, key)
				delete(s.nicknames, producer.info.NickName)
			}, func() {
				producer.state = Canceled
				s.canceledProducers[key] = producer
				delete(s.illegalProducers, key)
				s.nicknames[producer.info.NickName] = struct{}{}
			})
			continue
		}
	}
}

// ProcessIllegalBlockEvidence takes a illegal block payload and change the
// producers state immediately.  This is a spacial case that can be handled
// before it packed into a block.
func (s *State) ProcessSpecialTxPayload(p types.Payload) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	if inactivePayload, ok := p.(*payload.InactiveArbitrators); ok {
		s.processEmergencyInactiveArbitrators(inactivePayload, 0)
	} else {
		s.processIllegalEvidence(p, 0)
	}

	// Commit changes here if no errors found.
	s.history.commit(0)
}

// tryLeaveInactiveMode for inactive arbitrators to try leaving inactive mode if
// over the inactive duration
func (s *State) tryLeaveInactiveMode(blockHeight uint32) {
	tryLeaveInactive := func(key string, producer *Producer) {
		s.history.append(blockHeight, func() {
			if blockHeight > producer.inactiveSince+s.chainParams.InactiveDuration {
				pk, err := common.HexStringToBytes(key)
				if err != nil {
					log.Warn("[tryLeaveInactiveMode] convert to public key" +
						" byte error")
					return
				}

				producer.state = Activate

				if !s.arbiters.IsCRCArbitrator(pk) {
					s.activityProducers[key] = producer
				}
				delete(s.inactiveProducers, key)
			}
		}, func() {
			if blockHeight <= producer.inactiveSince+s.chainParams.InactiveDuration {

				producer.state = Inactivate
				s.inactiveProducers[key] = producer
				delete(s.activityProducers, key)
			}
		})
	}

	for k, v := range s.inactiveProducers {
		tryLeaveInactive(k, v)
	}
}

// countArbitratorsInactivity count arbitrators inactive rounds, and change to
// inactive if more than "MaxInactiveRounds"
func (s *State) countArbitratorsInactivity(height uint32, arbitrators []string) {

	countInactiveRounds := func(key string, producer *Producer) {
		s.history.append(height, func() {

			if producer.state == Activate {

				if producer.inactiveRounds == 0 {
					producer.inactiveSince = height
				}
				producer.inactiveRounds++

				if producer.inactiveRounds >= s.chainParams.MaxInactiveRounds {

					producer.state = Inactivate
					s.inactiveProducers[key] = producer
					delete(s.activityProducers, key)

					producer.penalty += s.chainParams.InactivePenalty
				}
			}
		}, func() {

			if producer.state == Inactivate {
				producer.inactiveRounds--

				if producer.inactiveRounds < s.chainParams.MaxInactiveRounds {

					producer.state = Activate
					s.activityProducers[key] = producer
					delete(s.inactiveProducers, key)

					if producer.penalty < s.chainParams.InactivePenalty {
						producer.penalty = common.Fixed64(0)
					} else {
						producer.penalty -= s.chainParams.InactivePenalty
					}
				}
			} else if producer.state == Activate {

				producer.inactiveRounds--
				if producer.inactiveRounds == 0 {
					producer.inactiveSince = math.MaxUint32
				}
			}
		})
	}

	for _, v := range arbitrators {
		if _, ok := s.activityProducers[v]; ok {
			countInactiveRounds(v, s.activityProducers[v])
		} else {
			log.Warn("unknown active producer: ", v)
		}
	}
}

// RollbackTo restores the database state to the given height, if no enough
// history to rollback to return error.
func (s *State) RollbackTo(height uint32) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()
	return s.history.rollbackTo(height)
}

// GetHistory returns a history state instance storing the producers and votes
// on the historical height.
func (s *State) GetHistory(height uint32) (*State, error) {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	// Seek to state to target height.
	if err := s.history.seekTo(height); err != nil {
		return nil, err
	}

	// Take a snapshot of the history.
	return s.snapshot(), nil
}

// snapshot takes a snapshot of current state and returns the copy.
func (s *State) snapshot() *State {
	state := State{
		pendingProducers:  make(map[string]*Producer),
		activityProducers: make(map[string]*Producer),
		inactiveProducers: make(map[string]*Producer),
		canceledProducers: make(map[string]*Producer),
		illegalProducers:  make(map[string]*Producer),
	}
	copyMap(state.pendingProducers, s.pendingProducers)
	copyMap(state.activityProducers, s.activityProducers)
	copyMap(state.inactiveProducers, s.inactiveProducers)
	copyMap(state.canceledProducers, s.canceledProducers)
	copyMap(state.illegalProducers, s.illegalProducers)
	return &state
}

// GetSnapshot returns a snapshot of the state according to the given height.
func (s *State) GetSnapshot(height uint32) *State {
	s.mtx.RLock()
	defer s.mtx.RUnlock()
	offset := (s.history.height - height) / snapshotInterval
	index := (s.cursor - 1 - int(offset) + maxSnapshots) % maxSnapshots
	return s.snapshots[index]
}

// copyMap copy the src map's key, value pairs into dst map.
func copyMap(dst map[string]*Producer, src map[string]*Producer) {
	for k, v := range src {
		p := *v
		dst[k] = &p
	}
}

// NewState returns a new State instance.
func NewState(arbiters interfaces.Arbitrators, chainParams *config.Params) *State {
	return &State{
		arbiters:          arbiters,
		chainParams:       chainParams,
		nodeOwnerKeys:     make(map[string]string),
		pendingProducers:  make(map[string]*Producer),
		activityProducers: make(map[string]*Producer),
		inactiveProducers: make(map[string]*Producer),
		canceledProducers: make(map[string]*Producer),
		illegalProducers:  make(map[string]*Producer),
		votes:             make(map[string]*types.Output),
		nicknames:         make(map[string]struct{}),
		history:           newHistory(maxHistoryCapacity),
	}
}
