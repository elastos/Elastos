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
	"github.com/elastos/Elastos.ELA/common/log"
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

	// Inactivate indicates the producer has been inactive for a period which shall
	// be punished and will be activate later
	Inactivate

	// Canceled indicates the producer was canceled.
	Canceled

	// FoundBad indicates the producer was found doing bad.
	FoundBad

	// ReturnedDeposit indicates the producer has canceled and returned deposit
	ReturnedDeposit
)

// producerStateStrings is a array of producer states back to their constant
// names for pretty printing.
var producerStateStrings = []string{"Pending", "Activate", "Inactivate",
	"Canceled", "FoundBad", "ReturnedDeposit"}

func (ps ProducerState) String() string {
	if int(ps) < len(producerStateStrings) {
		return producerStateStrings[ps]
	}
	return fmt.Sprintf("ProducerState-%d", ps)
}

// Producer holds a producer's info.  It provides read only methods to access
// producer's info.
type Producer struct {
	info                  payload.ProducerInfo
	state                 ProducerState
	registerHeight        uint32
	cancelHeight          uint32
	inactiveSince         uint32
	activateRequestHeight uint32
	penalty               common.Fixed64
	votes                 common.Fixed64
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

	mtx                 sync.RWMutex
	nodeOwnerKeys       map[string]string // NodePublicKey as key, OwnerPublicKey as value
	pendingProducers    map[string]*Producer
	activityProducers   map[string]*Producer
	inactiveProducers   map[string]*Producer
	canceledProducers   map[string]*Producer
	illegalProducers    map[string]*Producer
	onDutyMissingCounts map[string]uint32
	votes               map[string]*types.Output
	nicknames           map[string]struct{}
	specialTxHashes     map[string]struct{}
	history             *history

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
	if producer, ok := s.inactiveProducers[key]; ok {
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

func (s *State) GetArbiters() interfaces.Arbitrators {
	return s.arbiters
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

func (s *State) GetInterfaceProducers() []interfaces.Producer {
	s.mtx.RLock()
	producers := s.getInterfaceProducers()
	s.mtx.RUnlock()
	return producers
}

func (s *State) getInterfaceProducers() []interfaces.Producer {
	producers := make([]interfaces.Producer, 0, len(s.activityProducers))
	for _, producer := range s.activityProducers {
		producers = append(producers, producer)
	}
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
	ok := s.isInactiveProducer(publicKey)
	s.mtx.RUnlock()
	return ok
}

func (s *State) isInactiveProducer(publicKey []byte) bool {
	_, ok := s.inactiveProducers[s.getProducerKey(publicKey)]
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

// SpecialTxExists returns if a special tx (typically means illegal and
// inactive tx) is exists by it's hash
func (s *State) SpecialTxExists(hash *common.Uint256) bool {
	s.mtx.RLock()
	_, ok := s.specialTxHashes[hash.String()]
	s.mtx.RLock()
	return ok
}

// IsDPOSTransaction returns if a transaction will change the producers and
// votes state.
func (s *State) IsDPOSTransaction(tx *types.Transaction) bool {
	s.mtx.RLock()
	defer s.mtx.RUnlock()

	switch tx.TxType {
	// Transactions will changes the producers state.
	case types.RegisterProducer, types.UpdateProducer, types.CancelProducer,
		types.ActivateProducer, types.IllegalProposalEvidence,
		types.IllegalVoteEvidence, types.IllegalBlockEvidence,
		types.IllegalSidechainEvidence:
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
func (s *State) ProcessBlock(block *types.Block, confirm *payload.Confirm) {
	// get on duty arbiter before lock in case of recurrent lock
	onDutyArbitrator := s.arbiters.GetOnDutyArbitratorByHeight(block.Height)
	arbitersCount := s.arbiters.GetArbitersCount()

	s.mtx.Lock()
	defer s.mtx.Unlock()

	s.processTransactions(block.Transactions, block.Height)

	if confirm != nil {
		s.countArbitratorsInactivity(block.Height, arbitersCount,
			onDutyArbitrator, confirm)
	}

	s.processArbitrators(block, block.Height)

	// Take snapshot when snapshot point arrives.
	if (block.Height-s.chainParams.VoteStartHeight)%snapshotInterval == 0 {
		s.cursor = s.cursor % maxSnapshots
		s.snapshots[s.cursor] = s.snapshot()
		s.cursor++
	}

	// Commit changes here if no errors found.
	s.history.commit(block.Height)
}

func (s *State) processArbitrators(block *types.Block, height uint32) {
	s.history.append(height, func() {
		s.arbiters.IncreaseChainHeight(block.Height)
	}, func() {
		s.arbiters.DecreaseChainHeight(block.Height)
	})
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

	// Check if any pending producers has got 6 confirms, set them to activate.
	activateProducerFromInactive := func(key string, producer *Producer) {
		s.history.append(height, func() {
			producer.state = Activate
			s.activityProducers[key] = producer
			delete(s.inactiveProducers, key)
		}, func() {
			producer.state = Inactivate
			s.inactiveProducers[key] = producer
			delete(s.activityProducers, key)
		})
	}

	if len(s.pendingProducers) > 0 {
		for key, producer := range s.pendingProducers {
			if height-producer.registerHeight+1 >= 6 {
				activateProducerFromPending(key, producer)
			}
		}

	}
	if len(s.inactiveProducers) > 0 {
		for key, producer := range s.inactiveProducers {
			if height > producer.activateRequestHeight &&
				height-producer.activateRequestHeight+1 >= 6 {
				activateProducerFromInactive(key, producer)
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
		s.cancelProducer(tx.Payload.(*payload.ProcessProducer),
			height)

	case types.ActivateProducer:
		s.activateProducer(tx.Payload.(*payload.ProcessProducer), height)

	case types.TransferAsset:
		s.processVotes(tx, height)

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
	}
}

// registerProducer handles the register producer transaction.
func (s *State) registerProducer(payload *payload.ProducerInfo, height uint32) {
	nickname := payload.NickName
	nodeKey := hex.EncodeToString(payload.NodePublicKey)
	ownerKey := hex.EncodeToString(payload.OwnerPublicKey)
	producer := Producer{
		info:                  *payload,
		registerHeight:        height,
		votes:                 0,
		inactiveSince:         0,
		penalty:               common.Fixed64(0),
		activateRequestHeight: math.MaxUint32,
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
func (s *State) cancelProducer(payload *payload.ProcessProducer, height uint32) {
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

// activateProducer handles the activate producer transaction.
func (s *State) activateProducer(p *payload.ProcessProducer, height uint32) {
	producer := s.getProducer(p.OwnerPublicKey)
	s.history.append(height, func() {
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

func (s *State) returnDeposit(tx *types.Transaction, height uint32) {

	returnAction := func(producer *Producer) {
		s.history.append(height, func() {
			producer.state = ReturnedDeposit
		}, func() {
			producer.state = Canceled
		})
	}

	for _, program := range tx.Programs {
		pk := program.Code[1 : len(program.Code)-1]
		if producer := s.GetProducer(pk); producer != nil && producer.state == Canceled {
			returnAction(producer)
		}
	}
}

// processEmergencyInactiveArbitrators change producer state according to
// emergency inactive arbitrators
func (s *State) processEmergencyInactiveArbitrators(
	inactivePayload *payload.InactiveArbitrators, height uint32) {

	addEmergencyInactiveArbitrator := func(key string, producer *Producer) {
		s.history.append(height, func() {
			s.setInactiveProducer(producer, key, height)
		}, func() {
			s.revertSettingInactiveProducer(producer, key, height)
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
					registerHeight:        height,
					votes:                 0,
					inactiveSince:         0,
					penalty:               common.Fixed64(0),
					activateRequestHeight: math.MaxUint32,
				}
				addEmergencyInactiveArbitrator(pkStr, producer)

			} else {
				log.Warn("unknown active producer: ", v)
			}
		}
	}

}

// recordSpecialTx record hash of a special tx
func (s *State) recordSpecialTx(tx *types.Transaction, height uint32) {
	s.history.append(height, func() {
		s.specialTxHashes[tx.Hash().String()] = struct{}{}
	}, func() {
		delete(s.specialTxHashes, tx.Hash().String())
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

// setInactiveProducer set active producer to inactive state
func (s *State) setInactiveProducer(producer *Producer, key string,
	height uint32) {
	producer.inactiveSince = height
	producer.state = Inactivate
	s.inactiveProducers[key] = producer
	delete(s.activityProducers, key)

	producer.penalty += s.chainParams.InactivePenalty
}

// revertSettingInactiveProducer revert operation about setInactiveProducer
func (s *State) revertSettingInactiveProducer(producer *Producer, key string,
	height uint32) {
	producer.inactiveSince = 0
	producer.state = Activate
	s.activityProducers[key] = producer
	delete(s.inactiveProducers, key)

	if producer.penalty < s.chainParams.InactivePenalty {
		producer.penalty = common.Fixed64(0)
	} else {
		producer.penalty -= s.chainParams.InactivePenalty
	}
}

// countArbitratorsInactivity count arbitrators inactive rounds, and change to
// inactive if more than "MaxInactiveRounds"
func (s *State) countArbitratorsInactivity(height, totalArbitersCount uint32,
	onDutyArbiter []byte, confirm *payload.Confirm) {
	// check inactive arbitrators after producers has participated in
	if int(totalArbitersCount) == len(s.chainParams.CRCArbiters) {
		return
	}

	key := s.getProducerKey(onDutyArbiter)

	isMissOnDuty := !bytes.Equal(onDutyArbiter, confirm.Proposal.Sponsor)
	if producer, ok := s.activityProducers[key]; ok {
		count, existMissingRecord := s.onDutyMissingCounts[key]

		s.history.append(height, func() {
			if isMissOnDuty {
				if existMissingRecord {
					s.onDutyMissingCounts[key]++

					if s.onDutyMissingCounts[key] >=
						s.chainParams.MaxInactiveRounds {
						s.setInactiveProducer(producer, key, height)
						delete(s.onDutyMissingCounts, key)
					}
				} else {
					s.onDutyMissingCounts[key] = 1
				}
			} else {
				if existMissingRecord {
					delete(s.onDutyMissingCounts, key)
				}
			}
		}, func() {
			if isMissOnDuty {
				if existMissingRecord {
					if producer.state == Inactivate {
						s.revertSettingInactiveProducer(producer, key, height)
						s.onDutyMissingCounts[key] = s.chainParams.MaxInactiveRounds
					} else {
						if count > 1 {
							s.onDutyMissingCounts[key]--
						} else {
							delete(s.onDutyMissingCounts, key)
						}
					}
				}
			} else {
				if existMissingRecord {
					s.onDutyMissingCounts[key] = count
				}
			}
		})
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
		pendingProducers:    make(map[string]*Producer),
		activityProducers:   make(map[string]*Producer),
		inactiveProducers:   make(map[string]*Producer),
		canceledProducers:   make(map[string]*Producer),
		illegalProducers:    make(map[string]*Producer),
		onDutyMissingCounts: make(map[string]uint32),
	}
	copyMap(state.pendingProducers, s.pendingProducers)
	copyMap(state.activityProducers, s.activityProducers)
	copyMap(state.inactiveProducers, s.inactiveProducers)
	copyMap(state.canceledProducers, s.canceledProducers)
	copyMap(state.illegalProducers, s.illegalProducers)
	copyCountMap(state.onDutyMissingCounts, s.onDutyMissingCounts)
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

// copyMap copy the src map's key, value pairs into dst map.
func copyCountMap(dst map[string]uint32, src map[string]uint32) {
	for k, v := range src {
		dst[k] = v
	}
}

// NewState returns a new State instance.
func NewState(arbiters interfaces.Arbitrators, chainParams *config.Params) *State {
	return &State{
		arbiters:            arbiters,
		chainParams:         chainParams,
		nodeOwnerKeys:       make(map[string]string),
		pendingProducers:    make(map[string]*Producer),
		activityProducers:   make(map[string]*Producer),
		inactiveProducers:   make(map[string]*Producer),
		canceledProducers:   make(map[string]*Producer),
		illegalProducers:    make(map[string]*Producer),
		onDutyMissingCounts: make(map[string]uint32),
		votes:               make(map[string]*types.Output),
		nicknames:           make(map[string]struct{}),
		specialTxHashes:     make(map[string]struct{}),
		history:             newHistory(maxHistoryCapacity),
	}
}
