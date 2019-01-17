package state

import (
	"encoding/hex"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
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

	// Canceled indicates the producer was canceled.
	Canceled

	// FoundBad indicates the producer was found doing bad.
	FoundBad
)

// producerStateStrings is a array of producer states back to their constant
// names for pretty printing.
var producerStateStrings = []string{"Pending", "Activate", "Canceled", "FoundBad"}

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

// maxHistoryCapacity indicates the maximum capacity of change history.
const maxHistoryCapacity = 10

// State is a memory database storing DPOS producers state, like pending
// producers active producers and there votes.
type State struct {
	mtx               sync.RWMutex
	pendingProducers  map[string]*Producer
	activityProducers map[string]*Producer
	canceledProducers map[string]*Producer
	illegalProducers  map[string]*Producer
	votes             map[string]*types.Output
	nicknames         map[string]struct{}
	history           *history
}

// getProducer returns a producer if public key matches any registered producer.
func (s *State) getProducer(publicKey []byte) *Producer {
	key := hex.EncodeToString(publicKey)
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

// GetProducer returns a producer if public key matches any registered producer,
// including canceled and illegal producers.
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

// IsPendingProducer returns if a producer is in pending list according to the
// public key.
func (s *State) IsPendingProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.pendingProducers[hex.EncodeToString(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsActiveProducer returns if a producer is in activate list according to the
// public key.
func (s *State) IsActiveProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.activityProducers[hex.EncodeToString(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsCanceledProducer returns if a producer is in canceled list according to the
// public key.
func (s *State) IsCanceledProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.canceledProducers[hex.EncodeToString(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsIllegalProducer returns if a producer is in illegal list according to the
// public key.
func (s *State) IsIllegalProducer(publicKey []byte) bool {
	s.mtx.RLock()
	_, ok := s.illegalProducers[hex.EncodeToString(publicKey)]
	s.mtx.RUnlock()
	return ok
}

// IsUnusedNickname returns if a nickname is unused.
func (s *State) IsUnusedNickname(nickname string) bool {
	s.mtx.RLock()
	_, ok := s.nicknames[nickname]
	s.mtx.RUnlock()
	return !ok
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

			// Cancel votes.
			for _, input := range tx.Inputs {
				_, ok := s.votes[input.ReferKey()]
				if ok {
					return true
				}
			}
		}
	}

	return false
}

// ProcessTransactions takes the transactions and the height when they have been
// packed into a block.  Then loop through the transactions to update producers
// state and votes according to transactions content.
func (s *State) ProcessTransactions(txs []*types.Transaction, height uint32) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

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

	// Commit changes here if no errors found.
	s.history.commit(height)
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
	}
}

// registerProducer handles the register producer transaction, returns if this
// registration is valid.
func (s *State) registerProducer(payload *payload.ProducerInfo, height uint32) {
	nickname := payload.NickName
	key := hex.EncodeToString(payload.PublicKey)
	s.history.append(height, func() {
		s.nicknames[nickname] = struct{}{}
		s.pendingProducers[key] =
			&Producer{info: *payload, registerHeight: height, votes: 0}
	}, func() {
		delete(s.nicknames, nickname)
		delete(s.pendingProducers, key)
	})
}

// updateProducer handles the update producer transaction, returns if this
// update is valid.
func (s *State) updateProducer(info *payload.ProducerInfo, height uint32) {
	producer := s.getProducer(info.PublicKey)
	producerInfo := producer.info
	s.history.append(height, func() {
		delete(s.nicknames, producerInfo.NickName)
		producer.info = *info
		s.nicknames[info.NickName] = struct{}{}
	}, func() {
		delete(s.nicknames, info.NickName)
		producer.info = producerInfo
		delete(s.nicknames, producerInfo.NickName)
	})
}

// cancelProducer handles the cancel producer transaction, returns if this
// cancel is valid.
func (s *State) cancelProducer(payload *payload.CancelProducer, height uint32) {
	key := hex.EncodeToString(payload.PublicKey)
	producer := s.getProducer(payload.PublicKey)
	s.history.append(height, func() {
		producer.state = Canceled
		producer.cancelHeight = height
		s.canceledProducers[key] = producer
		delete(s.activityProducers, key)
		delete(s.nicknames, producer.info.NickName)
	}, func() {
		producer.state = Activate
		producer.cancelHeight = 0
		s.activityProducers[key] = producer
		delete(s.canceledProducers, key)
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

		// Cancel votes.
		for _, input := range tx.Inputs {
			output, ok := s.votes[input.ReferKey()]
			if ok {
				s.processVoteCancel(output, height)
			}
		}
	}
}

// processVoteOutput takes a transaction output with vote payload, validate the
// payload and increase producers votes.
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
func (s *State) ProcessIllegalBlockEvidence(payload types.Payload) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	s.processIllegalEvidence(payload, 0)

	// Commit changes here if no errors found.
	s.history.commit(0)
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

	// Make a copy of the state.
	state := State{
		pendingProducers:  make(map[string]*Producer),
		activityProducers: make(map[string]*Producer),
		canceledProducers: make(map[string]*Producer),
		illegalProducers:  make(map[string]*Producer),
	}
	copyMap(state.pendingProducers, s.pendingProducers)
	copyMap(state.activityProducers, s.activityProducers)
	copyMap(state.canceledProducers, s.canceledProducers)
	copyMap(state.illegalProducers, s.illegalProducers)

	return &state, nil
}

// copyMap copy the src map's key, value pairs into dst map.
func copyMap(dst map[string]*Producer, src map[string]*Producer) {
	for k, v := range src {
		dst[k] = v
	}
}

// NewState returns a new State instance.
func NewState() *State {
	return &State{
		pendingProducers:  make(map[string]*Producer),
		activityProducers: make(map[string]*Producer),
		canceledProducers: make(map[string]*Producer),
		illegalProducers:  make(map[string]*Producer),
		votes:             make(map[string]*types.Output),
		nicknames:         make(map[string]struct{}),
		history:           newHistory(maxHistoryCapacity),
	}
}
