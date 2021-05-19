package manager

import (
	"bytes"
	"errors"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/dtime"
	"github.com/elastos/Elastos.ELA/dpos/log"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/dpos/store"
)

type ProposalDispatcherConfig struct {
	store.EventStoreAnalyzerConfig
	EventMonitor *log.EventMonitor
	Consensus    *Consensus
	Network      DPOSNetwork
	Manager      *DPOSManager
	Account      account.Account
	ChainParams  *config.Params
	TimeSource   dtime.MedianTimeSource
}

type ProposalDispatcher struct {
	cfg ProposalDispatcherConfig

	processingBlock     *types.Block
	processingProposal  *payload.DPOSProposal
	acceptVotes         map[common.Uint256]*payload.DPOSProposalVote
	rejectedVotes       map[common.Uint256]*payload.DPOSProposalVote
	pendingProposals    map[common.Uint256]*payload.DPOSProposal
	precociousProposals map[common.Uint256]*payload.DPOSProposal
	pendingVotes        map[common.Uint256]*payload.DPOSProposalVote

	proposalProcessFinished bool
	crcBadNetwork           bool
	firstBadNetworkRecover  bool

	inactiveCountDown           ViewChangesCountDown
	currentInactiveArbitratorTx *types.Transaction
	signedTxs                   map[common.Uint256]interface{}

	eventAnalyzer  *store.EventStoreAnalyzer
	illegalMonitor *IllegalBehaviorMonitor
}

func (p *ProposalDispatcher) RequestAbnormalRecovering() {
	height := blockchain.DefaultLedger.Blockchain.GetHeight()
	msgItem := &dmsg.RequestConsensus{Height: height}
	log.Info("[RequestAbnormalRecovering] broadcast message to peers")
	p.cfg.Network.BroadcastMessage(msgItem)
}

func (p *ProposalDispatcher) GetProcessingBlock() *types.Block {
	return p.processingBlock
}

func (p *ProposalDispatcher) GetProcessingProposal() *payload.DPOSProposal {
	return p.processingProposal
}

func (p *ProposalDispatcher) ProcessVote(v *payload.DPOSProposalVote,
	accept bool) (succeed bool, finished bool) {
	log.Info("[ProcessVote] start")
	defer log.Info("[ProcessVote] end")

	if err := blockchain.VoteCheck(v); err != nil {
		log.Warn("Invalid vote: ", err.Error())
		return false, false
	}

	if p.alreadyExistVote(v) {
		log.Info("Already has vote")
		return false, false
	}

	if anotherVote, legal := p.illegalMonitor.IsLegalVote(v); !legal {
		p.illegalMonitor.ProcessIllegalVote(v, anotherVote)
		return
	}

	if accept {
		return p.countAcceptedVote(v)
	} else {
		return p.countRejectedVote(v)
	}
}

func (p *ProposalDispatcher) AddPendingVote(v *payload.DPOSProposalVote) {
	p.pendingVotes[v.Hash()] = v
}

func (p *ProposalDispatcher) StartProposal(b *types.Block) {
	log.Info("[StartProposal] start")
	defer log.Info("[StartProposal] end")

	if p.processingBlock != nil {
		log.Info("[StartProposal] start proposal failed")
		return
	}
	p.processingBlock = b

	//p.cfg.Network.BroadcastMessage(dmsg.NewInventory(b.Hash()))
	proposal := &payload.DPOSProposal{Sponsor: p.cfg.Manager.GetPublicKey(),
		BlockHash: b.Hash(), ViewOffset: p.cfg.Consensus.GetViewOffset()}
	var err error
	proposal.Sign, err = p.cfg.Account.SignProposal(proposal)
	if err != nil {
		log.Error("[StartProposal] start proposal failed:", err.Error())
		return
	}

	log.Info("[StartProposal] sponsor:", p.cfg.Manager.GetPublicKey())

	m := &dmsg.Proposal{
		Proposal: *proposal,
	}

	log.Info("[StartProposal] send proposal message finished, Proposal Hash: ", dmsg.GetMessageHash(m))
	p.cfg.Network.BroadcastMessage(m)

	proposalEvent := log.ProposalEvent{
		Sponsor:      common.BytesToHexString(proposal.Sponsor),
		BlockHash:    proposal.BlockHash,
		ReceivedTime: p.cfg.TimeSource.AdjustedTime(),
		ProposalHash: proposal.Hash(),
		RawData:      proposal,
		Result:       false,
	}
	p.cfg.EventMonitor.OnProposalArrived(&proposalEvent)
	p.acceptProposal(proposal)
}

func (p *ProposalDispatcher) TryStartSpeculatingProposal(b *types.Block) {
	log.Info("[TryStartSpeculatingProposal] start")
	defer log.Info("[TryStartSpeculatingProposal] end")

	if p.processingBlock != nil {
		log.Warn("[TryStartSpeculatingProposal] processingBlock is not nil")
		return
	}
	p.processingBlock = b
}

func (p *ProposalDispatcher) FinishProposal() bool {
	log.Info("[FinishProposal] start")
	defer log.Info("[FinishProposal] end")

	if p.processingBlock == nil {
		log.Warn("[FinishProposal] nil processing block")
		return false
	}

	proposal, blockHash := p.processingProposal.Sponsor, p.processingBlock.Hash()

	p.AppendConfirm()

	proposalEvent := log.ProposalEvent{
		Sponsor:   common.BytesToHexString(proposal),
		BlockHash: blockHash,
		EndTime:   p.cfg.TimeSource.AdjustedTime(),
		Result:    true,
	}
	p.cfg.EventMonitor.OnProposalFinished(&proposalEvent)
	p.FinishConsensus()

	return true
}

func (p *ProposalDispatcher) CleanProposals(changeView bool) {
	log.Info("Clean proposals")

	p.illegalMonitor.Reset(changeView)

	p.processingBlock = nil
	p.processingProposal = nil
	p.acceptVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	p.rejectedVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	p.pendingVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	p.proposalProcessFinished = false
	if !changeView {
		p.inactiveCountDown.Reset(0)
		p.currentInactiveArbitratorTx = nil
		p.signedTxs = map[common.Uint256]interface{}{}
		p.pendingProposals = make(map[common.Uint256]*payload.DPOSProposal)
		p.precociousProposals = make(map[common.Uint256]*payload.DPOSProposal)

		p.eventAnalyzer.Clear()
	} else {
		// clear pending proposals less than current view offset
		currentOffset := p.cfg.Consensus.GetViewOffset()
		for k, v := range p.pendingProposals {
			if v.ViewOffset < currentOffset {
				delete(p.pendingProposals, k)
			}
		}
		for k, v := range p.precociousProposals {
			if v.ViewOffset < currentOffset {
				delete(p.precociousProposals, k)
			}
		}
	}
}

func (p *ProposalDispatcher) ResetByCurrentView() {
	p.illegalMonitor.Reset(false)

	p.processingBlock = nil
	p.processingProposal = nil
	p.acceptVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	p.rejectedVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	p.pendingVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	p.proposalProcessFinished = false

	p.inactiveCountDown.Reset(p.cfg.Consensus.GetViewOffset() + 1)
	p.currentInactiveArbitratorTx = nil
	p.signedTxs = map[common.Uint256]interface{}{}
	p.pendingProposals = make(map[common.Uint256]*payload.DPOSProposal)
	p.precociousProposals = make(map[common.Uint256]*payload.DPOSProposal)

	p.eventAnalyzer.Clear()

}

func (p *ProposalDispatcher) ProcessProposal(id peer.PID, d *payload.DPOSProposal,
	force bool) (needRecord bool, handled bool) {
	log.Info("[ProcessProposal] start")
	defer log.Info("[ProcessProposal] end")

	self := bytes.Equal(id[:], d.Sponsor)
	if err := blockchain.ProposalCheck(d); err != nil {
		log.Warn("invalid proposal: ", err.Error())
		return false, true
	}

	if p.IsViewChangedTimeOut() {
		log.Info("enter emergency state, proposal will be discard")
		return true, !self
	}

	if p.processingProposal != nil && d.Hash().IsEqual(
		p.processingProposal.Hash()) {
		log.Info("already processing proposal")
		return true, true
	}

	if _, err := blockchain.DefaultLedger.Blockchain.GetBlockByHash(d.BlockHash); err == nil {
		log.Info("already exist block in block chain")
		return true, !self
	}

	if d.ViewOffset != p.cfg.Consensus.GetViewOffset() {
		log.Info("have different view offset")
		if d.ViewOffset > p.cfg.Consensus.GetViewOffset() {
			p.precociousProposals[d.Hash()] = d
		}
		return true, !self
	}

	if !force {
		if _, ok := p.pendingProposals[d.Hash()]; ok {
			log.Info("already have proposal, wait for processing")
			return true, true
		}
	}

	if anotherProposal, ok := p.illegalMonitor.IsLegalProposal(d); !ok {
		p.illegalMonitor.ProcessIllegalProposal(d, anotherProposal)
		return true, true
	}

	if !p.cfg.Consensus.IsArbitratorOnDuty(d.Sponsor) {
		currentArbiter := p.cfg.Manager.GetArbitrators().GetNextOnDutyArbitrator(p.cfg.Consensus.GetViewOffset())
		log.Info("viewOffset:", p.cfg.Consensus.GetViewOffset(), "current arbiter:",
			common.BytesToHexString(currentArbiter), "sponsor:", d.Sponsor)
		p.rejectProposal(d)
		log.Warn("reject: current arbiter is not sponsor")
		return true, !self
	}

	currentBlock, ok := p.cfg.Manager.GetBlockCache().TryGetValue(d.BlockHash)
	if !ok || !p.cfg.Consensus.IsRunning() {
		p.pendingProposals[d.Hash()] = d
		p.cfg.Manager.OnInv(id, d.BlockHash)
		log.Info("received pending proposal")
		return true, true
	} else {
		p.TryStartSpeculatingProposal(currentBlock)
	}

	if currentBlock.Height != p.processingBlock.Height {
		log.Warn("[ProcessProposal] Invalid block height")
		return true, !self
	}

	if !d.BlockHash.IsEqual(p.processingBlock.Hash()) {
		log.Warn("[ProcessProposal] Invalid block hash")
		return true, !self
	}

	if !p.proposalProcessFinished {
		p.acceptProposal(d)
	}

	return true, true
}

func (p *ProposalDispatcher) AppendConfirm() {
	currentVoteSlot := &payload.Confirm{
		Proposal: *p.processingProposal,
		Votes:    make([]payload.DPOSProposalVote, 0),
	}
	for _, v := range p.acceptVotes {
		currentVoteSlot.Votes = append(currentVoteSlot.Votes, *v)
	}

	log.Info("[AppendConfirm] append confirm.")
	go func() {
		if _, _, err := p.cfg.Manager.AppendConfirm(
			currentVoteSlot); err != nil {
			log.Warn("[AppendConfirm] append failed: ", err)
		}
	}()
}

func (p *ProposalDispatcher) OnBlockAdded(b *types.Block) {
	for k, v := range p.pendingProposals {
		if p.cfg.Consensus.IsRunning() && v.BlockHash.IsEqual(b.Hash()) {
			// block is already exist, will not use PID, given PID{} is ok
			if needRecord, _ := p.ProcessProposal(
				peer.PID{}, v, true); needRecord {
				p.illegalMonitor.AddProposal(v)
			}
			delete(p.pendingProposals, k)
		}
	}
}

func (p *ProposalDispatcher) UpdatePrecociousProposals() {
	for k, v := range p.precociousProposals {
		if p.cfg.Consensus.IsRunning() &&
			v.ViewOffset == p.cfg.Consensus.GetViewOffset() {
			if needRecord, _ := p.ProcessProposal(
				peer.PID{}, v, true); needRecord {
				p.illegalMonitor.AddProposal(v)
			}
			delete(p.precociousProposals, k)
		}
	}
}

func (p *ProposalDispatcher) FinishConsensus() {
	if p.cfg.Consensus.IsRunning() {
		log.Info("[FinishConsensus] start")
		defer log.Info("[FinishConsensus] end")

		p.cfg.Manager.changeOnDuty()
		height := blockchain.DefaultLedger.Blockchain.GetHeight()
		c := log.ConsensusEvent{EndTime: p.cfg.TimeSource.AdjustedTime(), Height: height}
		p.cfg.EventMonitor.OnConsensusFinished(&c)
		p.cfg.Consensus.SetReady()
		p.CleanProposals(false)
	}
}

func (p *ProposalDispatcher) CollectConsensusStatus(status *dmsg.ConsensusStatus) error {
	status.AcceptVotes = make([]payload.DPOSProposalVote, 0, len(p.acceptVotes))
	for _, v := range p.acceptVotes {
		status.AcceptVotes = append(status.AcceptVotes, *v)
	}

	status.RejectedVotes = make([]payload.DPOSProposalVote, 0, len(p.rejectedVotes))
	for _, v := range p.rejectedVotes {
		status.RejectedVotes = append(status.RejectedVotes, *v)
	}

	status.PendingProposals = make([]payload.DPOSProposal, 0, len(p.pendingProposals))
	for _, v := range p.pendingProposals {
		status.PendingProposals = append(status.PendingProposals, *v)
	}

	status.PendingVotes = make([]payload.DPOSProposalVote, 0, len(p.pendingVotes))
	for _, v := range p.pendingVotes {
		status.PendingVotes = append(status.PendingVotes, *v)
	}

	return nil
}

func (p *ProposalDispatcher) RecoverFromConsensusStatus(status *dmsg.ConsensusStatus) error {
	p.acceptVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	for _, v := range status.AcceptVotes {
		vote := v
		p.acceptVotes[v.Hash()] = &vote
	}

	p.rejectedVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	for _, v := range status.RejectedVotes {
		vote := v
		p.rejectedVotes[v.Hash()] = &vote
	}

	p.pendingProposals = make(map[common.Uint256]*payload.DPOSProposal)
	for _, v := range status.PendingProposals {
		vote := v
		p.pendingProposals[v.Hash()] = &vote
	}

	p.pendingVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
	for _, v := range status.PendingVotes {
		vote := v
		p.pendingVotes[v.Hash()] = &vote
	}

	if status.ConsensusStatus == consensusReady {
		p.processingBlock = nil
	}
	return nil
}

func (p *ProposalDispatcher) IsCRCBadNetWork() bool {
	peers := p.cfg.Network.GetActivePeers()
	var count int
	for _, v := range peers {
		pid := v.PID()
		if p.cfg.Arbitrators.IsCRCArbitrator(pid[:]) {
			count++
		}
	}
	return count <= p.cfg.Arbitrators.GetCRCArbitersCount()*2/3
}

func (p *ProposalDispatcher) IsViewChangedTimeOut() bool {
	if p.crcBadNetwork {
		if !p.IsCRCBadNetWork() {
			p.crcBadNetwork = false
			if p.firstBadNetworkRecover {
				p.firstBadNetworkRecover = false
				return false
			}
			p.ResetByCurrentView()
		}
		return false
	}

	if p.IsCRCBadNetWork() {
		p.crcBadNetwork = true
		return false
	}

	return p.inactiveCountDown.IsTimeOut()
}

func (p *ProposalDispatcher) OnIllegalBlocksTxReceived(i *payload.DPOSIllegalBlocks) {
	p.inactiveCountDown.SetEliminated(i.Hash())
}

func (p *ProposalDispatcher) OnInactiveArbitratorsReceived(id peer.PID,
	tx *types.Transaction) {
	if _, ok := p.signedTxs[tx.Hash()]; ok {
		log.Warn("[OnInactiveArbitratorsReceived] already processed")
		return
	}

	log.Info("[OnInactiveArbitratorsReceived] received inactive tx")

	if !p.IsViewChangedTimeOut() {
		log.Warn("[OnInactiveArbitratorsReceived] received inactive" +
			" arbitrators transaction when normal view changing")
		return
	}

	inactivePayload := tx.Payload.(*payload.InactiveArbitrators)
	if len(inactivePayload.Arbitrators) == 0 {
		log.Warn("[OnInactiveArbitratorsReceived] received empty payload")
		return
	}
	if err := p.checkInactivePayloadContent(inactivePayload); err != nil {
		log.Warn("[OnInactiveArbitratorsReceived] error: ", err)
		return
	}

	p.signedTxs[tx.Hash()] = nil

	response := &dmsg.ResponseInactiveArbitrators{
		TxHash: tx.Hash(),
		Signer: p.cfg.Manager.GetPublicKey(),
	}
	var err error
	if response.Sign, err = p.cfg.Account.SignTx(tx); err != nil {
		log.Warn("[OnInactiveArbitratorsReceived] sign response message"+
			" error, details: ", err.Error())
	}
	if err := p.cfg.Network.SendMessageToPeer(id, response); err != nil {
		log.Warn("[OnInactiveArbitratorsReceived] send msg error: ", err)
	}

	log.Info("[OnInactiveArbitratorsReceived] response inactive tx sign")
}

func (p *ProposalDispatcher) checkInactivePayloadContent(
	inactivePayload *payload.InactiveArbitrators) error {
	// todo pass this check for now
	return nil

	inactiveArbitratorsMap := make(map[string]interface{})
	for _, v := range p.eventAnalyzer.ParseInactiveArbitrators() {
		inactiveArbitratorsMap[v] = nil
	}
	if len(inactivePayload.Arbitrators) != len(inactiveArbitratorsMap) {
		return errors.New("received inactive arbitrators transaction " +
			"with wrong arbitrators count")
	}
	for _, v := range inactivePayload.Arbitrators {
		if _, exist := inactiveArbitratorsMap[common.BytesToHexString(
			v)]; !exist {
			return errors.New("disagree with inactive arbitrators")
		}
	}
	return nil
}

func (p *ProposalDispatcher) OnResponseInactiveArbitratorsReceived(
	txHash *common.Uint256, signer []byte, sign []byte) {
	log.Info("[OnResponseInactiveArbitratorsReceived] collect transaction" +
		" signs")

	if p.currentInactiveArbitratorTx == nil ||
		!p.currentInactiveArbitratorTx.Hash().IsEqual(*txHash) {
		log.Warn("[OnResponseInactiveArbitratorsReceived] unknown " +
			"inactive arbitrators transaction")
		return
	}

	data := new(bytes.Buffer)
	if err := p.currentInactiveArbitratorTx.SerializeUnsigned(
		data); err != nil {
		log.Warn("[OnResponseInactiveArbitratorsReceived] transaction "+
			"serialize error, details: ", err)
		return
	}

	pk, err := crypto.DecodePoint(signer)
	if err != nil {
		log.Warn("[OnResponseInactiveArbitratorsReceived] decode signer "+
			"error, details: ", err)
		return
	}

	if err := crypto.Verify(*pk, data.Bytes(), sign); err != nil {
		log.Warn("[OnResponseInactiveArbitratorsReceived] sign verify "+
			"error, details: ", err)
		return
	}

	pro := p.currentInactiveArbitratorTx.Programs[0]
	buf := new(bytes.Buffer)
	buf.Write(pro.Parameter)
	buf.WriteByte(byte(len(sign)))
	buf.Write(sign)
	pro.Parameter = buf.Bytes()

	p.tryEnterEmergencyState(len(pro.Parameter) / crypto.SignatureScriptLength)
}

func (p *ProposalDispatcher) tryEnterEmergencyState(signCount int) bool {
	log.Info("[tryEnterEmergencyState] current sign count: ", signCount)

	minSignCount := int(float64(len(p.cfg.Arbitrators.GetCRCArbiters()))*
		state.MajoritySignRatioNumerator/state.MajoritySignRatioDenominator) + 1
	if signCount >= minSignCount {
		payload := p.currentInactiveArbitratorTx.Payload.(*payload.InactiveArbitrators)
		p.illegalMonitor.AddEvidence(payload)
		p.cfg.Manager.AppendToTxnPool(p.currentInactiveArbitratorTx)

		if err := p.cfg.Arbitrators.ProcessSpecialTxPayload(
			p.currentInactiveArbitratorTx.Payload,
			blockchain.DefaultLedger.Blockchain.GetHeight()); err != nil {
			log.Error("[tryEnterEmergencyState] force change arbitrators"+
				" error: ", err.Error())
			return false
		}
		p.cfg.Manager.clearInactiveData(payload)

		log.Info("[tryEnterEmergencyState] successfully entered emergency"+
			" state ", payload.Hash())
		return true
	}

	return false
}

func (p *ProposalDispatcher) alreadyExistVote(v *payload.DPOSProposalVote) bool {
	_, ok := p.acceptVotes[v.Hash()]
	if ok {
		log.Info("[alreadyExistVote]: ", v.Signer, "already in the AcceptVotes!")
		return true
	}

	_, ok = p.rejectedVotes[v.Hash()]
	if ok {
		log.Info("[alreadyExistVote]: ", v.Signer, "already in the RejectedVotes!")
		return true
	}

	return false
}

func (p *ProposalDispatcher) countAcceptedVote(v *payload.DPOSProposalVote) (
	succeed bool, finished bool) {
	log.Info("[countAcceptedVote] start")
	defer log.Info("[countAcceptedVote] end")

	if v.Accept {
		log.Info("[countAcceptedVote] Received needed sign, collect it into AcceptVotes!")
		p.acceptVotes[v.Hash()] = v

		if p.cfg.Manager.GetArbitrators().HasArbitersMajorityCount(len(p.acceptVotes)) {
			log.Info("Collect majority signs, finish proposal.")
			return true, p.FinishProposal()
		}
		return true, false
	}

	return false, false
}

func (p *ProposalDispatcher) countRejectedVote(v *payload.DPOSProposalVote) (
	succeed bool, finished bool) {
	log.Info("[countRejectedVote] start")
	defer log.Info("[countRejectedVote] end")

	if !v.Accept {
		log.Info("[countRejectedVote] Received invalid sign, collect it into RejectedVotes!")
		p.rejectedVotes[v.Hash()] = v

		if p.cfg.Manager.GetArbitrators().HasArbitersMinorityCount(len(p.rejectedVotes)) {
			p.CleanProposals(true)
			p.cfg.Consensus.ChangeView()
			return true, true
		}
		return true, false
	}

	return false, false
}

func (p *ProposalDispatcher) acceptProposal(d *payload.DPOSProposal) {
	log.Info("[acceptProposal] start")
	defer log.Info("[acceptProposal] end")

	p.setProcessingProposal(d)
	if !p.cfg.Manager.isCurrentArbiter() {
		return
	}

	vote := &payload.DPOSProposalVote{ProposalHash: d.Hash(),
		Signer: p.cfg.Manager.GetPublicKey(), Accept: true}
	var err error
	vote.Sign, err = p.cfg.Account.SignVote(vote)
	if err != nil {
		log.Error("[acceptProposal] sign failed")
		return
	}
	voteMsg := &dmsg.Vote{Command: dmsg.CmdAcceptVote, Vote: *vote}
	p.ProcessVote(vote, true)

	p.proposalProcessFinished = true
	p.cfg.Network.BroadcastMessage(voteMsg)
	log.Info("[acceptProposal] send acc_vote msg:", dmsg.GetMessageHash(voteMsg).String())

	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(vote.Signer),
		ReceivedTime: p.cfg.TimeSource.AdjustedTime(), Result: true, RawData: vote}
	p.cfg.EventMonitor.OnVoteArrived(&voteEvent)
	p.eventAnalyzer.AppendConsensusVote(vote)
}

func (p *ProposalDispatcher) rejectProposal(d *payload.DPOSProposal) {
	p.setProcessingProposal(d)

	vote := &payload.DPOSProposalVote{ProposalHash: d.Hash(),
		Signer: p.cfg.Manager.GetPublicKey(), Accept: false}
	var err error
	vote.Sign, err = p.cfg.Account.SignVote(vote)
	if err != nil {
		log.Error("[rejectProposal] sign failed")
		return
	}
	msg := &dmsg.Vote{Command: dmsg.CmdRejectVote, Vote: *vote}
	log.Info("[rejectProposal] send rej_vote msg:", dmsg.GetMessageHash(msg))

	_, ok := p.cfg.Manager.GetBlockCache().TryGetValue(d.BlockHash)
	if !ok {
		log.Error("[rejectProposal] can't find block")
		return
	}
	p.ProcessVote(vote, false)
	p.cfg.Network.BroadcastMessage(msg)

	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(vote.Signer),
		ReceivedTime: p.cfg.TimeSource.AdjustedTime(), Result: false, RawData: vote}
	p.cfg.EventMonitor.OnVoteArrived(&voteEvent)
	p.eventAnalyzer.AppendConsensusVote(vote)
}

func (p *ProposalDispatcher) setProcessingProposal(d *payload.DPOSProposal) {
	p.processingProposal = d

	for _, v := range p.pendingVotes {
		if v.ProposalHash.IsEqual(d.Hash()) {
			p.ProcessVote(v, v.Accept)
		}
	}
	p.pendingVotes = make(map[common.Uint256]*payload.DPOSProposalVote)
}

func (p *ProposalDispatcher) CreateInactiveArbitrators() (
	*types.Transaction, error) {
	var err error

	inactivePayload := &payload.InactiveArbitrators{
		Sponsor:     p.cfg.Manager.GetPublicKey(),
		Arbitrators: [][]byte{},
		BlockHeight: blockchain.DefaultLedger.Blockchain.GetHeight() + 1,
	}
	inactiveArbitrators := p.eventAnalyzer.ParseInactiveArbitrators()
	for _, v := range inactiveArbitrators {
		var pk []byte
		pk, err = common.HexStringToBytes(v)
		if err != nil {
			return nil, err
		}
		inactivePayload.Arbitrators = append(inactivePayload.Arbitrators, pk)
	}
	if len(inactivePayload.Arbitrators) == 0 {
		return nil, errors.New("found no inactive arbiters")
	}

	con := contract.Contract{Prefix: contract.PrefixMultiSig}
	if con.Code, err = p.createArbitratorsRedeemScript(); err != nil {
		return nil, err
	}

	programHash := con.ToProgramHash()
	tx := &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.InactiveArbitrators,
		PayloadVersion: payload.InactiveArbitratorsVersion,
		Payload:        inactivePayload,
		Attributes: []*types.Attribute{{
			Usage: types.Script,
			Data:  programHash.Bytes(),
		}},
		LockTime: 0,
		Outputs:  []*types.Output{},
		Inputs:   []*types.Input{},
		Fee:      0,
	}

	var sign []byte
	if sign, err = p.cfg.Account.SignTx(tx); err != nil {
		return nil, err
	}
	parameter := append([]byte{byte(len(sign))}, sign...)
	tx.Programs = []*program.Program{
		{
			Code:      con.Code,
			Parameter: parameter,
		},
	}

	p.currentInactiveArbitratorTx = tx
	return tx, nil
}

func (p *ProposalDispatcher) createArbitratorsRedeemScript() ([]byte, error) {
	var pks []*crypto.PublicKey
	for _, v := range p.cfg.Arbitrators.GetCRCArbiters() {
		pk, err := crypto.DecodePoint(v)
		if err != nil {
			return nil, err
		}
		pks = append(pks, pk)
	}

	arbitratorsCount := p.cfg.Arbitrators.GetCRCArbitersCount()
	minSignCount := int(float64(arbitratorsCount)*
		state.MajoritySignRatioNumerator/state.MajoritySignRatioDenominator) + 1
	return contract.CreateMultiSigRedeemScript(minSignCount, pks)
}

func NewDispatcherAndIllegalMonitor(cfg ProposalDispatcherConfig) (
	*ProposalDispatcher, *IllegalBehaviorMonitor) {
	p := &ProposalDispatcher{
		cfg:                    cfg,
		processingBlock:        nil,
		processingProposal:     nil,
		acceptVotes:            make(map[common.Uint256]*payload.DPOSProposalVote),
		rejectedVotes:          make(map[common.Uint256]*payload.DPOSProposalVote),
		pendingProposals:       make(map[common.Uint256]*payload.DPOSProposal),
		precociousProposals:    make(map[common.Uint256]*payload.DPOSProposal),
		pendingVotes:           make(map[common.Uint256]*payload.DPOSProposalVote),
		signedTxs:              make(map[common.Uint256]interface{}),
		firstBadNetworkRecover: true,
		eventAnalyzer: store.NewEventStoreAnalyzer(store.EventStoreAnalyzerConfig{
			Store:       cfg.Store,
			Arbitrators: cfg.Arbitrators,
		}),
	}
	p.inactiveCountDown = ViewChangesCountDown{
		dispatcher:      p,
		consensus:       cfg.Consensus,
		arbitrators:     cfg.Arbitrators,
		timeoutRefactor: 0,
	}
	p.inactiveCountDown.Reset(0)

	i := &IllegalBehaviorMonitor{
		dispatcher:      p,
		cachedProposals: make(map[common.Uint256]*payload.DPOSProposal),
		evidenceCache: evidenceCache{
			make(map[common.Uint256]payload.DPOSIllegalData)},
		manager: cfg.Manager,
	}
	p.illegalMonitor = i
	return p, i
}
