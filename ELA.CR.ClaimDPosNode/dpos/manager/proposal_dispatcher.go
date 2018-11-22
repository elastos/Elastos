package manager

import (
	"bytes"
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	msg2 "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/node"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type ProposalDispatcher interface {
	AbnormalRecovering

	//status
	GetProcessingBlock() *core.Block
	GetProcessingProposal() *core.DPosProposal
	IsProcessingBlockEmpty() bool
	CurrentHeight() uint32

	//proposal
	StartProposal(b *core.Block)
	CleanProposals()
	FinishProposal()
	TryStartSpeculatingProposal(b *core.Block)
	ProcessProposal(d core.DPosProposal)

	FinishConsensus()

	ProcessVote(v core.DPosProposalVote, accept bool)
	AddPendingVote(v core.DPosProposalVote)

	OnAbnormalStateDetected()
	RequestAbnormalRecovering()
	TryAppendAndBroadcastConfirmBlockMsg() bool
}

type proposalDispatcher struct {
	processingBlock    *core.Block
	processingProposal *core.DPosProposal
	acceptVotes        map[common.Uint256]core.DPosProposalVote
	rejectedVotes      map[common.Uint256]core.DPosProposalVote
	pendingProposals   map[common.Uint256]core.DPosProposal
	pendingVotes       map[common.Uint256]core.DPosProposalVote

	eventMonitor *log.EventMonitor
	consensus    Consensus
	network      DposNetwork
	manager      DposManager
	account      account.DposAccount
}

func (p *proposalDispatcher) OnAbnormalStateDetected() {
	p.RequestAbnormalRecovering()
}

func (p *proposalDispatcher) RequestAbnormalRecovering() {
	height := p.CurrentHeight()
	msgItem := &msg2.RequestConsensus{Height: height}
	peerID := p.network.GetActivePeer()
	if peerID == nil {
		log.Error("[RequestAbnormalRecovering] can not find active peer")
		return
	}
	p.network.SendMessageToPeer(*peerID, msgItem)
}

func (p *proposalDispatcher) GetProcessingBlock() *core.Block {
	return p.processingBlock
}

func (p *proposalDispatcher) GetProcessingProposal() *core.DPosProposal {
	return p.processingProposal
}

func (p *proposalDispatcher) ProcessVote(v core.DPosProposalVote, accept bool) {
	if accept {
		p.countAcceptedVote(v)
	} else {
		p.countRejectedVote(v)
	}
}

func (p *proposalDispatcher) AddPendingVote(v core.DPosProposalVote) {
	p.pendingVotes[v.Hash()] = v
}

func (p *proposalDispatcher) IsProcessingBlockEmpty() bool {
	return p.processingBlock == nil
}

func (p *proposalDispatcher) StartProposal(b *core.Block) {
	if p.processingBlock != nil {
		log.Info("[StartProposal] start proposal failed")
		return
	}
	p.processingBlock = b

	proposal := core.DPosProposal{Sponsor: p.manager.GetPublicKey(), BlockHash: b.Hash(), ViewOffset: p.consensus.GetViewOffset()}
	var err error
	proposal.Sign, err = p.account.SignProposal(&proposal)
	if err != nil {
		log.Error("[StartProposal] start proposal failed:", err.Error())
		return
	}

	log.Debug("[StartProposal] sponsor:", p.manager.GetPublicKey())

	m := &msg2.Proposal{
		Proposal: proposal,
	}

	log.Info("[StartProposal] send proposal message finished, Proposal Hash: ", msg2.GetMessageHash(m))
	p.network.BroadcastMessage(m)

	rawData := new(bytes.Buffer)
	proposal.Serialize(rawData)
	proposalEvent := log.ProposalEvent{
		Proposal:     proposal.Sponsor,
		BlockHash:    proposal.BlockHash,
		ReceivedTime: time.Now(),
		RawData:      rawData.Bytes(),
		Result:       false,
	}
	p.eventMonitor.OnProposalArrived(proposalEvent)

	p.acceptProposal(proposal)
}

func (p *proposalDispatcher) TryStartSpeculatingProposal(b *core.Block) {

	if p.processingBlock != nil {
		return
	}
	p.processingBlock = b
}

func (p *proposalDispatcher) FinishProposal() {
	proposal, blockHash := p.processingProposal.Sponsor, p.processingBlock.Hash()

	log.Info("[p.consensus.IsOnDuty()]", p.consensus.IsOnDuty())
	log.Info("[FinishProposal] try append and broad cast confirm block msg")

	if !p.TryAppendAndBroadcastConfirmBlockMsg() {
		log.Warn("Add block failed, no need to broadcast confirm message")
		return
	}

	p.FinishConsensus()

	proposalEvent := log.ProposalEvent{
		Proposal:  proposal,
		BlockHash: blockHash,
		EndTime:   time.Now(),
		Result:    true,
	}
	p.eventMonitor.OnProposalFinished(proposalEvent)
}

func (p *proposalDispatcher) CleanProposals() {
	log.Info("Clean proposals")
	p.processingBlock = nil
	p.processingProposal = nil
	p.acceptVotes = make(map[common.Uint256]core.DPosProposalVote)
	p.rejectedVotes = make(map[common.Uint256]core.DPosProposalVote)
	p.pendingVotes = make(map[common.Uint256]core.DPosProposalVote)

	//todo clear pending proposals that are lower than current consensus height
}

func (p *proposalDispatcher) ProcessProposal(d core.DPosProposal) {

	log.Info("[ProcessProposal] start")
	defer log.Info("[ProcessProposal] end")

	if !blockchain.IsProposalValid(&d) {
		log.Warn("Invalid proposal.")
		return
	}

	if !p.consensus.IsArbitratorOnDuty(d.Sponsor) {
		currentArbiter := blockchain.DefaultLedger.Arbitrators.GetNextOnDutyArbitrator(p.consensus.GetViewOffset())
		log.Info("viewOffset:", p.consensus.GetViewOffset(), "current arbiter:",
			common.BytesToHexString(currentArbiter), "sponsor:", d.Sponsor)
		p.rejectProposal(d)
		log.Warn("reject: current arbiter is not sponsor")
		return
	}

	currentBlock, ok := p.manager.GetBlockCache().TryGetValue(d.BlockHash)
	if !ok || !p.consensus.IsRunning() {
		p.pendingProposals[d.Hash()] = d
		log.Info("Received pending proposal.")
		return
	} else {
		p.TryStartSpeculatingProposal(currentBlock)
	}

	if currentBlock.Height != p.processingBlock.Height {
		log.Warn("[ProcessProposal] Invalid block height")
		return
	}

	if !d.BlockHash.IsEqual(p.processingBlock.Hash()) {
		log.Warn("[ProcessProposal] Invalid block hash")
		return
	}

	p.acceptProposal(d)
}

func (p *proposalDispatcher) TryAppendAndBroadcastConfirmBlockMsg() bool {
	currentVoteSlot := &core.DPosProposalVoteSlot{
		Hash:     p.processingBlock.Hash(),
		Proposal: *p.processingProposal,
		Votes:    make([]core.DPosProposalVote, 0),
	}
	for _, v := range p.acceptVotes {
		currentVoteSlot.Votes = append(currentVoteSlot.Votes, v)
	}

	log.Info("[TryAppendAndBroadcastConfirmBlockMsg][OnDuty],append confirm.")
	if err := node.LocalNode.AppendConfirm(currentVoteSlot); err != nil {
		return false
	}

	return true
}

func (p *proposalDispatcher) OnBlockAdded(b *core.Block) {

	if p.consensus.IsRunning() {
		for k, v := range p.pendingProposals {
			if v.BlockHash.IsEqual(b.Hash()) {
				p.ProcessProposal(v)
				delete(p.pendingProposals, k)
				break
			}
		}
	}
}

func (p *proposalDispatcher) FinishConsensus() {
	log.Info("[FinishConsensus], change states to ConsensusReady")
	c := log.ConsensusEvent{EndTime: time.Now(), Height: p.CurrentHeight()}
	p.eventMonitor.OnConsensusFinished(c)
	p.consensus.SetReady()
	p.CleanProposals()
}

func (p *proposalDispatcher) CollectConsensusStatus(height uint32, status *msg2.ConsensusStatus) error {
	if height > p.CurrentHeight() {
		return errors.New("Requesting height greater than current processing height")
	}

	if p.processingBlock != nil {
		status.ProcessingBlock = *p.processingBlock
	}
	if p.processingProposal != nil {
		status.ProcessingProposal = *p.processingProposal
	}

	status.AcceptVotes = make([]core.DPosProposalVote, 0, len(p.acceptVotes))
	for _, v := range p.acceptVotes {
		status.AcceptVotes = append(status.AcceptVotes, v)
	}

	status.RejectedVotes = make([]core.DPosProposalVote, 0, len(p.rejectedVotes))
	for _, v := range p.rejectedVotes {
		status.RejectedVotes = append(status.RejectedVotes, v)
	}

	status.PendingProposals = make([]core.DPosProposal, 0, len(p.pendingProposals))
	for _, v := range p.pendingProposals {
		status.PendingProposals = append(status.PendingProposals, v)
	}

	status.PendingVotes = make([]core.DPosProposalVote, 0, len(p.pendingVotes))
	for _, v := range p.pendingVotes {
		status.PendingVotes = append(status.PendingVotes, v)
	}

	return nil
}

func (p *proposalDispatcher) RecoverFromConsensusStatus(status *msg2.ConsensusStatus) error {
	if status.ProcessingBlock.Height < p.CurrentHeight() {
		return errors.New("Recovering height less than current processing height")
	}

	p.processingBlock = &status.ProcessingBlock
	p.processingProposal = &status.ProcessingProposal

	p.acceptVotes = make(map[common.Uint256]core.DPosProposalVote)
	for _, v := range status.AcceptVotes {
		p.acceptVotes[v.Hash()] = v
	}

	p.rejectedVotes = make(map[common.Uint256]core.DPosProposalVote)
	for _, v := range status.RejectedVotes {
		p.rejectedVotes[v.Hash()] = v
	}

	p.pendingProposals = make(map[common.Uint256]core.DPosProposal)
	for _, v := range status.PendingProposals {
		p.pendingProposals[v.Hash()] = v
	}

	p.pendingVotes = make(map[common.Uint256]core.DPosProposalVote)
	for _, v := range status.PendingVotes {
		p.pendingVotes[v.Hash()] = v
	}

	return nil
}

func (p *proposalDispatcher) CurrentHeight() uint32 {
	var height uint32
	currentBlock := p.GetProcessingBlock()
	if currentBlock != nil {
		height = currentBlock.Height
	} else {
		height = blockchain.DefaultLedger.Blockchain.BlockHeight
	}
	return height
}

func (p *proposalDispatcher) alreadyExistVote(v core.DPosProposalVote) bool {
	_, ok := p.acceptVotes[v.Hash()]
	if ok {
		log.Info("[alreadyExistVote]: ", v.Signer, "aready in the AcceptVotes!")
		return true
	}

	_, ok = p.rejectedVotes[v.Hash()]
	if ok {
		log.Info("[alreadyExistVote]: ", v.Signer, "aready in the RejectedVotes!")
		return true
	}

	return false
}

func (p *proposalDispatcher) countAcceptedVote(v core.DPosProposalVote) {
	log.Info("[countAcceptedVote] start")
	defer log.Info("[countAcceptedVote] end")

	if v.Accept && blockchain.IsVoteValid(&v) && !p.alreadyExistVote(v) {
		log.Info("[countAcceptedVote] Received needed sign, collect it into AcceptVotes!")
		p.acceptVotes[v.Hash()] = v

		if blockchain.DefaultLedger.Arbitrators.HasArbitersMajorityCount(uint32(len(p.acceptVotes))) {
			log.Info("Collect majority signs, finish proposal.")
			p.FinishProposal()
		}
	}
}

func (p *proposalDispatcher) countRejectedVote(v core.DPosProposalVote) {
	log.Info("[countRejectedVote] start")
	defer log.Info("[countRejectedVote] end")

	if !v.Accept && blockchain.IsVoteValid(&v) && !p.alreadyExistVote(v) {
		log.Info("[countRejectedVote] Received invalid sign, collect it into RejectedVotes!")
		p.rejectedVotes[v.Hash()] = v

		if blockchain.DefaultLedger.Arbitrators.HasArbitersMinorityCount(uint32(len(p.rejectedVotes))) {
			p.CleanProposals()
			p.consensus.ChangeView()
		}
	}
}

func (p *proposalDispatcher) acceptProposal(d core.DPosProposal) {
	p.setProcessingProposal(d)

	vote := core.DPosProposalVote{ProposalHash: d.Hash(), Signer: p.manager.GetPublicKey(), Accept: true}
	var err error
	vote.Sign, err = p.account.SignVote(&vote)
	if err != nil {
		log.Error("[acceptProposal] sign failed")
		return
	}
	voteMsg := &msg2.Vote{Command: msg2.CmdAcceptVote, Vote: vote}

	p.ProcessVote(vote, true)
	p.network.BroadcastMessage(voteMsg)
	log.Info("[acceptProposal] send acc_vote msg:", msg2.GetMessageHash(voteMsg).String())

	rawData := new(bytes.Buffer)
	vote.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: vote.Signer, ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	p.eventMonitor.OnVoteArrived(voteEvent)
}

func (p *proposalDispatcher) rejectProposal(d core.DPosProposal) {
	p.setProcessingProposal(d)

	vote := core.DPosProposalVote{ProposalHash: d.Hash(), Signer: p.manager.GetPublicKey(), Accept: false}
	var err error
	vote.Sign, err = p.account.SignVote(&vote)
	if err != nil {
		log.Error("[rejectProposal] sign failed")
		return
	}
	msg := &msg2.Vote{Command: msg2.CmdRejectVote, Vote: vote}
	log.Info("[rejectProposal] send rej_vote msg:", msg2.GetMessageHash(msg))

	_, ok := p.manager.GetBlockCache().TryGetValue(d.BlockHash)
	if !ok {
		log.Error("[rejectProposal] can't find block")
		return
	}
	p.ProcessVote(vote, false)
	p.network.BroadcastMessage(msg)

	rawData := new(bytes.Buffer)
	vote.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: vote.Signer, ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	p.eventMonitor.OnVoteArrived(voteEvent)
}

func (p *proposalDispatcher) setProcessingProposal(d core.DPosProposal) {
	p.processingProposal = &d

	for _, v := range p.pendingVotes {
		if v.ProposalHash.IsEqual(d.Hash()) {
			if v.Accept {
				p.countAcceptedVote(v)
			} else {
				p.countRejectedVote(v)
			}
		}
	}
	p.pendingVotes = make(map[common.Uint256]core.DPosProposalVote)
}

func NewDispatcher(consensus Consensus, eventMonitor *log.EventMonitor, network DposNetwork, manager DposManager, dposAccount account.DposAccount) ProposalDispatcher {
	p := &proposalDispatcher{
		processingBlock:    nil,
		processingProposal: nil,
		acceptVotes:        make(map[common.Uint256]core.DPosProposalVote),
		rejectedVotes:      make(map[common.Uint256]core.DPosProposalVote),
		pendingProposals:   make(map[common.Uint256]core.DPosProposal),
		pendingVotes:       make(map[common.Uint256]core.DPosProposalVote),
		eventMonitor:       eventMonitor,
		consensus:          consensus,
		network:            network,
		manager:            manager,
		account:            dposAccount,
	}
	return p
}
