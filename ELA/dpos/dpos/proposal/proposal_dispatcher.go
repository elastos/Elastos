package proposal

import (
	"bytes"
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/handler"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

type ProposalDispatcher struct {
	processingBlock  *core.Block
	currentVoteSlot  *core.DPosProposalVoteSlot
	acceptVotes      []core.DPosProposalVote
	rejectedVotes    []core.DPosProposalVote
	pendingProposals []core.DPosProposal

	eventMonitor *log.EventMonitor
	consensus    IConsensus
}

func (p *ProposalDispatcher) Initialize(consensus IConsensus, eventMonitor *log.EventMonitor) {
	p.consensus = consensus
	p.eventMonitor = eventMonitor
}

func (p *ProposalDispatcher) OnAbnormalStateDetected() {
	p.RequestAbnormalRecovering()
}

func (p *ProposalDispatcher) RequestAbnormalRecovering() {
	height := p.CurrentHeight()
	msg := &cs.RequestConsensusMessage{Command: cs.RequestConsensus, Height: height}
	peer, err := cs.P2PClientSingleton.PeerHandler.GetLastActivePeer()
	if err != nil {
		log.Error("[RequestAbnormalRecovering]", err.Error())
		return
	}
	peer.SendMessage(msg, nil)
}

func (p *ProposalDispatcher) GetProcessingBlock() *core.Block {
	return p.processingBlock
}

func (p *ProposalDispatcher) ProcessVote(v core.DPosProposalVote, accept bool) {
	if accept {
		p.countAcceptedVote(v)
	} else {
		p.countRejectedVote(v)
	}
}

func (p *ProposalDispatcher) IsVoteSlotEmpty() bool {
	return p.currentVoteSlot == nil || len(p.currentVoteSlot.Votes) == 0
}

func (p *ProposalDispatcher) StartProposal(b *core.Block) {
	if p.processingBlock != nil {
		log.Info("[StartProposal] start proposal failed")
		return
	}
	p.processingBlock = b
	p.currentVoteSlot = &core.DPosProposalVoteSlot{Hash: b.Hash(), Votes: make([]core.DPosProposalVote, 0)}

	proposal := core.DPosProposal{Sponsor: ArbitratorSingleton.Name, BlockHash: b.Hash()}
	proposal.Sign = proposal.SignProposal()

	log.Debug("[StartProposal] sponsor:", ArbitratorSingleton.Name)

	msg := &cs.ProposalMessage{
		Command:  cs.ReceivedProposal,
		Proposal: proposal,
	}

	log.Info("[StartProposal] send proposal message finished, Proposal Hash: ", cs.P2PClientSingleton.GetMessageHash(msg))
	cs.P2PClientSingleton.PeerHandler.SendAll(msg)

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

func (p *ProposalDispatcher) TryStartSpeculatingProposal(b *core.Block) {

	if p.processingBlock != nil {
		return
	}
	p.processingBlock = b
	p.currentVoteSlot = &core.DPosProposalVoteSlot{Hash: b.Hash(), Votes: make([]core.DPosProposalVote, 0)}
}

func (p *ProposalDispatcher) FinishProposal() {
	proposal, blockHash := p.acceptVotes[0].Proposal.Sponsor, p.processingBlock.Hash()

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

func (p *ProposalDispatcher) CleanProposals() {
	log.Info("Clean proposals")
	p.processingBlock = nil
	p.currentVoteSlot = nil
	p.acceptVotes = make([]core.DPosProposalVote, 0)
	p.rejectedVotes = make([]core.DPosProposalVote, 0)

	//todo clear pending proposals that are lower than current consensus height
}

func (p *ProposalDispatcher) ProcessProposal(d core.DPosProposal) {

	log.Info("[ProcessProposal] start")
	defer log.Info("[ProcessProposal] end")

	if !d.IsValid() {
		log.Warn("Invalid proposal.")
		return
	}

	if !p.consensus.IsArbitratorOnDuty(d.Sponsor) {
		currentArbiter, err := blockchain.GetNextOnDutyArbiter(p.consensus.GetViewOffset())
		if err != nil {
			log.Error(err)
		} else {
			log.Info("viewOffset:", p.consensus.GetViewOffset(), "current arbiter:",
				common.BytesToHexString(currentArbiter), "sponsor:", d.Sponsor)
		}
		p.rejectProposal(d)
		log.Warn("reject: current arbiter is not sponsor")
		return
	}

	currentBlock, ok := ArbitratorSingleton.BlockCache.TryGetValue(d.BlockHash)
	if !ok || !p.consensus.IsRunning() {
		p.pendingProposals = append(p.pendingProposals, d)
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

func (p *ProposalDispatcher) TryAppendAndBroadcastConfirmBlockMsg() bool {
	p.currentVoteSlot.Votes = make([]core.DPosProposalVote, 0)
	for _, v := range p.acceptVotes {
		p.currentVoteSlot.Votes = append(p.currentVoteSlot.Votes, v)
	}

	log.Debug("[TryAppendAndBroadcastConfirmBlockMsg] len signs:", len(p.currentVoteSlot.Votes))
	confirmMsg := msg.NewConfirm(p.currentVoteSlot)
	log.Info("[TryAppendAndBroadcastConfirmBlockMsg][OnDuty], broadcast ReceivedConfirm msg to confirm the block.")

	if err := blockchain.DefaultLedger.AppendBlocksAndConfirms([]*core.Block{p.processingBlock}, []*core.DPosProposalVoteSlot{p.currentVoteSlot}); err != nil {
		cs.P2PClientSingleton.AddMessageHash(cs.P2PClientSingleton.GetMessageHash(confirmMsg), p.CurrentHeight())
		cs.P2PClientSingleton.Server.BroadcastMessage(confirmMsg)
		log.Info("[TryAppendAndBroadcastConfirmBlockMsg][OnDuty], broadcast ReceivedConfirm msg to confirm the block. ok")
		return true
	}

	return false
}

func (p *ProposalDispatcher) OnBlockAdded(b *core.Block) {

	if p.consensus.IsRunning() {
		for i, v := range p.pendingProposals {
			if v.BlockHash.IsEqual(b.Hash()) {
				p.ProcessProposal(v)
				p.pendingProposals = append(p.pendingProposals[0:i], p.pendingProposals[i+1:]...)
				break
			}
		}
	}
}

func (p *ProposalDispatcher) FinishConsensus() {
	log.Info("[FinishConsensus], change states to ConsensusReady")
	c := log.ConsensusEvent{EndTime: time.Now(), Height: p.CurrentHeight()}
	p.eventMonitor.OnConsensusFinished(c)
	p.consensus.SetReady()
	p.CleanProposals()
}

func (p *ProposalDispatcher) OnSendPing(peer *peer.Peer) {
	p.consensus.ResponseHeartBeat(peer, cs.Ping, p.CurrentHeight())
}

func (p *ProposalDispatcher) CollectConsensusStatus(height uint32, status *cs.ConsensusStatus) error {
	if height > p.CurrentHeight() {
		return errors.New("Requesting height greater than current processing height")
	}

	if p.processingBlock != nil {
		status.ProcessingBlock = *p.processingBlock
	}
	status.AcceptVotes = p.acceptVotes
	status.RejectedVotes = p.rejectedVotes
	status.PendingProposals = p.pendingProposals
	return nil
}

func (p *ProposalDispatcher) RecoverFromConsensusStatus(status *cs.ConsensusStatus) error {
	if status.ProcessingBlock.Height < p.CurrentHeight() {
		return errors.New("Recovering height less than current processing height")
	}

	p.processingBlock = &status.ProcessingBlock
	p.acceptVotes = status.AcceptVotes
	p.rejectedVotes = status.RejectedVotes
	p.pendingProposals = status.PendingProposals
	return nil
}

func (p *ProposalDispatcher) CurrentHeight() uint32 {
	var height uint32
	currentBlock := p.GetProcessingBlock()
	if currentBlock != nil {
		height = currentBlock.Height
	} else {
		height = blockchain.DefaultLedger.Blockchain.BlockHeight
	}
	return height
}

func (p *ProposalDispatcher) alreadyExistVote(v core.DPosProposalVote) bool {
	for _, item := range p.acceptVotes {
		if item.Signer == v.Signer {
			log.Info("[alreadyExistVote]: ", v.Signer, "aready in the AcceptVotes!")
			return true
		}
	}
	for _, item := range p.rejectedVotes {
		if item.Signer == v.Signer {
			log.Info("[alreadyExistVote]: ", v.Signer, "aready in the RejectedVotes!")
			return true
		}
	}
	return false
}

func (p *ProposalDispatcher) countAcceptedVote(v core.DPosProposalVote) {
	log.Info("[countAcceptedVote] start")
	defer log.Info("[countAcceptedVote] end")

	if v.Accept && v.IsValid() && !p.alreadyExistVote(v) {
		log.Info("[countAcceptedVote] Received needed sign, collect it into AcceptVotes!")
		p.acceptVotes = append(p.acceptVotes, v)

		if blockchain.HasArbitersMajorityCount(uint32(len(p.acceptVotes))) {
			log.Info("Collect majority signs, finish proposal.")
			p.FinishProposal()
		}
	}
}

func (p *ProposalDispatcher) countRejectedVote(v core.DPosProposalVote) {
	log.Info("[countRejectedVote] start")
	defer log.Info("[countRejectedVote] end")

	if !v.Accept && v.IsValid() && !p.alreadyExistVote(v) {
		log.Info("[countRejectedVote] Received invalid sign, collect it into RejectedVotes!")
		p.rejectedVotes = append(p.rejectedVotes, v)

		if blockchain.HasArbitersMinorityCount(uint32(len(p.rejectedVotes))) {
			p.CleanProposals()
			p.consensus.ChangeView()
		}
	}
}

func (p *ProposalDispatcher) acceptProposal(d core.DPosProposal) {
	vote := core.DPosProposalVote{Proposal: d, Signer: ArbitratorSingleton.Name, Accept: true}
	vote.Sign = vote.SignVote()
	voteMsg := &cs.VoteMessage{Command: cs.AcceptVote, Vote: vote}
	p.ProcessVote(vote, true)
	cs.P2PClientSingleton.PeerHandler.SendAll(voteMsg)
	log.Info("[acceptProposal] send acc_vote msg:", cs.P2PClientSingleton.GetMessageHash(voteMsg).String())

	rawData := new(bytes.Buffer)
	vote.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: vote.Signer, ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	p.eventMonitor.OnVoteArrived(voteEvent)
}

func (p *ProposalDispatcher) rejectProposal(d core.DPosProposal) {
	vote := core.DPosProposalVote{Proposal: d, Signer: ArbitratorSingleton.Name, Accept: false}
	vote.Sign = vote.SignVote()
	msg := &cs.VoteMessage{Command: cs.RejectVote, Vote: vote}
	log.Info("[rejectProposal] send rej_vote msg:", cs.P2PClientSingleton.GetMessageHash(msg))

	_, ok := ArbitratorSingleton.BlockCache.TryGetValue(d.BlockHash)
	if !ok {
		log.Error("[rejectProposal] can't find block")
		return
	}
	p.ProcessVote(vote, false)
	cs.P2PClientSingleton.PeerHandler.SendAll(msg)

	rawData := new(bytes.Buffer)
	vote.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: vote.Signer, ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	p.eventMonitor.OnVoteArrived(voteEvent)
}
