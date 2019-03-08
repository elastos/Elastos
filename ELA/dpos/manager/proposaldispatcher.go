package manager

import (
	"bytes"
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/store"
)

type ProposalDispatcherConfig struct {
	store.EventStoreAnalyzerConfig
	EventMonitor *log.EventMonitor
	Consensus    *Consensus
	Network      DPOSNetwork
	Manager      *DPOSManager
	Account      account.DposAccount
	ChainParams  *config.Params
}

type ProposalDispatcher struct {
	cfg ProposalDispatcherConfig

	processingBlock    *types.Block
	processingProposal *payload.DPOSProposal
	acceptVotes        map[common.Uint256]payload.DPOSProposalVote
	rejectedVotes      map[common.Uint256]payload.DPOSProposalVote
	pendingProposals   map[common.Uint256]payload.DPOSProposal
	pendingVotes       map[common.Uint256]payload.DPOSProposalVote

	proposalProcessFinished bool

	inactiveCountDown           ViewChangesCountDown
	currentInactiveArbitratorTx *types.Transaction

	eventAnalyzer  *store.EventStoreAnalyzer
	illegalMonitor *IllegalBehaviorMonitor
}

func (p *ProposalDispatcher) OnAbnormalStateDetected() {
	p.RequestAbnormalRecovering()
}

func (p *ProposalDispatcher) RequestAbnormalRecovering() {
	height := p.CurrentHeight()
	msgItem := &dmsg.RequestConsensus{Height: height}
	peerID := p.cfg.Network.GetActivePeer()
	if peerID == nil {
		log.Error("[RequestAbnormalRecovering] can not find active peer")
		return
	}
	p.cfg.Network.SendMessageToPeer(*peerID, msgItem)
}

func (p *ProposalDispatcher) GetProcessingBlock() *types.Block {
	return p.processingBlock
}

func (p *ProposalDispatcher) GetProcessingProposal() *payload.DPOSProposal {
	return p.processingProposal
}

func (p *ProposalDispatcher) ProcessVote(v payload.DPOSProposalVote, accept bool) (succeed bool, finished bool) {
	log.Info("[ProcessVote] start")
	defer log.Info("[ProcessVote] end")

	if !blockchain.IsVoteValid(&v) {
		log.Info("Invalid vote")
		return false, false
	}

	if p.alreadyExistVote(v) {
		log.Info("Already has vote")
		return false, false
	}

	if anotherVote, legal := p.illegalMonitor.IsLegalVote(&v); !legal {
		p.illegalMonitor.ProcessIllegalVote(&v, anotherVote)
		return
	}

	if accept {
		return p.countAcceptedVote(v)
	} else {
		return p.countRejectedVote(v)
	}

	return false, false
}

func (p *ProposalDispatcher) AddPendingVote(v payload.DPOSProposalVote) {
	p.pendingVotes[v.Hash()] = v
}

func (p *ProposalDispatcher) IsProcessingBlockEmpty() bool {
	return p.processingBlock == nil
}

func (p *ProposalDispatcher) StartProposal(b *types.Block) {
	log.Info("[StartProposal] start")
	defer log.Info("[StartProposal] end")

	if p.processingBlock != nil {
		log.Info("[StartProposal] start proposal failed")
		return
	}
	p.processingBlock = b

	p.cfg.Network.BroadcastMessage(dmsg.NewInventory(b.Hash()))
	proposal := payload.DPOSProposal{Sponsor: p.cfg.Manager.GetPublicKey(),
		BlockHash: b.Hash(), ViewOffset: p.cfg.Consensus.GetViewOffset()}
	var err error
	proposal.Sign, err = p.cfg.Account.SignProposal(&proposal)
	if err != nil {
		log.Error("[StartProposal] start proposal failed:", err.Error())
		return
	}

	log.Info("[StartProposal] sponsor:", p.cfg.Manager.GetPublicKey())

	m := &dmsg.Proposal{
		Proposal: proposal,
	}

	log.Info("[StartProposal] send proposal message finished, Proposal Hash: ", dmsg.GetMessageHash(m))
	p.cfg.Network.BroadcastMessage(m)

	proposalEvent := log.ProposalEvent{
		Sponsor:      common.BytesToHexString(proposal.Sponsor),
		BlockHash:    proposal.BlockHash,
		ReceivedTime: time.Now(),
		ProposalHash: proposal.Hash(),
		RawData:      &proposal,
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

	if !p.TryAppendAndBroadcastConfirmBlockMsg() {
		log.Warn("Add block failed, no need to broadcast confirm message")
		return false
	}

	proposalEvent := log.ProposalEvent{
		Sponsor:   common.BytesToHexString(proposal),
		BlockHash: blockHash,
		EndTime:   time.Now(),
		RawData:   p.processingProposal,
		Result:    true,
	}
	p.cfg.EventMonitor.OnProposalFinished(&proposalEvent)
	p.FinishConsensus()

	return true
}

func (p *ProposalDispatcher) CleanProposals(changeView bool) {
	log.Info("Clean proposals")

	//todo clear pending proposals that are lower than current consensus height
	p.illegalMonitor.Reset(changeView)

	p.processingBlock = nil
	p.processingProposal = nil
	p.acceptVotes = make(map[common.Uint256]payload.DPOSProposalVote)
	p.rejectedVotes = make(map[common.Uint256]payload.DPOSProposalVote)
	p.pendingVotes = make(map[common.Uint256]payload.DPOSProposalVote)
	p.proposalProcessFinished = false

	if !changeView {
		p.inactiveCountDown.Reset()
		p.currentInactiveArbitratorTx = nil
	}
}

func (p *ProposalDispatcher) ProcessProposal(d payload.DPOSProposal, force bool) {
	log.Info("[ProcessProposal] start")
	defer log.Info("[ProcessProposal] end")

	if p.IsViewChangedTimeOut() {
		log.Info("enter emergency state, proposal will be discard")
		return
	}

	if p.processingProposal != nil && d.Hash().IsEqual(
		p.processingProposal.Hash()) {
		log.Info("already processing proposal")
		return
	}

	if _, err := blockchain.DefaultLedger.Blockchain.GetBlockByHash(d.BlockHash); err == nil {
		log.Info("already exist block in block chain")
		return
	}

	if d.ViewOffset != p.cfg.Consensus.GetViewOffset() {
		log.Info("have different view offset")
		return
	}

	if !force {
		if _, ok := p.pendingProposals[d.Hash()]; ok {
			log.Info("already have proposal, wait for processing")
			return
		}
	}

	if !blockchain.IsProposalValid(&d) {
		log.Warn("invalid proposal.")
		return
	}

	p.illegalMonitor.AddProposal(d)
	if anotherProposal, ok := p.illegalMonitor.IsLegalProposal(&d); !ok {
		p.illegalMonitor.ProcessIllegalProposal(&d, anotherProposal)
		return
	}

	if !p.cfg.Consensus.IsArbitratorOnDuty(d.Sponsor) {
		currentArbiter := p.cfg.Manager.GetArbitrators().GetNextOnDutyArbitrator(p.cfg.Consensus.GetViewOffset())
		log.Info("viewOffset:", p.cfg.Consensus.GetViewOffset(), "current arbiter:",
			common.BytesToHexString(currentArbiter), "sponsor:", d.Sponsor)
		p.rejectProposal(d)
		log.Warn("reject: current arbiter is not sponsor")
		return
	}

	currentBlock, ok := p.cfg.Manager.GetBlockCache().TryGetValue(d.BlockHash)
	if !ok || !p.cfg.Consensus.IsRunning() {
		p.pendingProposals[d.Hash()] = d
		log.Info("received pending proposal")
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

	if !p.proposalProcessFinished {
		p.acceptProposal(d)
	}
}

func (p *ProposalDispatcher) TryAppendAndBroadcastConfirmBlockMsg() bool {
	currentVoteSlot := &payload.Confirm{
		Proposal: *p.processingProposal,
		Votes:    make([]payload.DPOSProposalVote, 0),
	}
	for _, v := range p.acceptVotes {
		currentVoteSlot.Votes = append(currentVoteSlot.Votes, v)
	}

	log.Info("[TryAppendAndBroadcastConfirmBlockMsg] append confirm.")
	inMainChain, isOrphan, err := p.cfg.Manager.AppendConfirm(currentVoteSlot)
	if err != nil || !inMainChain || isOrphan {
		log.Error("[AppendConfirm] err:", err.Error())
		return false
	}

	return true
}

func (p *ProposalDispatcher) OnBlockAdded(b *types.Block) {

	if p.cfg.Consensus.IsRunning() {
		for k, v := range p.pendingProposals {
			if v.BlockHash.IsEqual(b.Hash()) {
				p.ProcessProposal(v, true)
				delete(p.pendingProposals, k)
				break
			}
		}
	}
}

func (p *ProposalDispatcher) FinishConsensus() {
	if p.cfg.Consensus.IsRunning() {
		log.Info("[FinishConsensus] start")
		defer log.Info("[FinishConsensus] end")

		c := log.ConsensusEvent{EndTime: time.Now(), Height: p.CurrentHeight()}
		p.cfg.EventMonitor.OnConsensusFinished(&c)
		p.cfg.Consensus.SetReady()
		p.CleanProposals(false)
	}
}

func (p *ProposalDispatcher) CollectConsensusStatus(height uint32, status *dmsg.ConsensusStatus) error {
	if height > p.CurrentHeight() {
		return errors.New("Requesting height greater than current processing height")
	}

	status.AcceptVotes = make([]payload.DPOSProposalVote, 0, len(p.acceptVotes))
	for _, v := range p.acceptVotes {
		status.AcceptVotes = append(status.AcceptVotes, v)
	}

	status.RejectedVotes = make([]payload.DPOSProposalVote, 0, len(p.rejectedVotes))
	for _, v := range p.rejectedVotes {
		status.RejectedVotes = append(status.RejectedVotes, v)
	}

	status.PendingProposals = make([]payload.DPOSProposal, 0, len(p.pendingProposals))
	for _, v := range p.pendingProposals {
		status.PendingProposals = append(status.PendingProposals, v)
	}

	status.PendingVotes = make([]payload.DPOSProposalVote, 0, len(p.pendingVotes))
	for _, v := range p.pendingVotes {
		status.PendingVotes = append(status.PendingVotes, v)
	}

	return nil
}

func (p *ProposalDispatcher) RecoverFromConsensusStatus(status *dmsg.ConsensusStatus) error {
	p.acceptVotes = make(map[common.Uint256]payload.DPOSProposalVote)
	for _, v := range status.AcceptVotes {
		p.acceptVotes[v.Hash()] = v
	}

	p.rejectedVotes = make(map[common.Uint256]payload.DPOSProposalVote)
	for _, v := range status.RejectedVotes {
		p.rejectedVotes[v.Hash()] = v
	}

	p.pendingProposals = make(map[common.Uint256]payload.DPOSProposal)
	for _, v := range status.PendingProposals {
		p.pendingProposals[v.Hash()] = v
	}

	p.pendingVotes = make(map[common.Uint256]payload.DPOSProposalVote)
	for _, v := range status.PendingVotes {
		p.pendingVotes[v.Hash()] = v
	}

	return nil
}

func (p *ProposalDispatcher) CurrentHeight() uint32 {
	var height uint32
	currentBlock := p.GetProcessingBlock()
	if currentBlock != nil {
		height = currentBlock.Height
	} else {
		height = blockchain.DefaultLedger.Blockchain.GetHeight()
	}
	return height
}

func (p *ProposalDispatcher) IsViewChangedTimeOut() bool {
	return p.inactiveCountDown.IsTimeOut()
}

func (p *ProposalDispatcher) OnInactiveArbitratorsReceived(
	tx *types.Transaction) {
	var err error

	if !p.IsViewChangedTimeOut() {
		log.Warn("[OnInactiveArbitratorsReceived] received inactive" +
			" arbitrators transaction when normal view changing")
		return
	}

	inactivePayload := tx.Payload.(*payload.InactiveArbitrators)
	if !p.cfg.Consensus.IsArbitratorOnDuty(inactivePayload.Sponsor) {
		log.Warn("[OnInactiveArbitratorsReceived] sender is not on duty")
		return
	}

	inactiveArbitratorsMap := make(map[string]interface{})
	for _, v := range p.eventAnalyzer.ParseInactiveArbitrators() {
		inactiveArbitratorsMap[v] = nil
	}
	for _, v := range inactivePayload.Arbitrators {
		if _, exist := inactiveArbitratorsMap[common.BytesToHexString(
			v)]; !exist {
			log.Warn("[OnInactiveArbitratorsReceived] disagree with " +
				"inactive arbitrators")
			return
		}
	}

	if !p.currentInactiveArbitratorTx.Hash().IsEqual(tx.Hash()) {
		p.currentInactiveArbitratorTx = tx
	}

	response := &dmsg.ResponseInactiveArbitrators{
		TxHash: tx.Hash(),
		Signer: p.cfg.Manager.GetPublicKey(),
	}
	if response.Sign, err = p.cfg.Account.SignTx(tx); err != nil {
		log.Warn("[OnInactiveArbitratorsReceived] sign response message"+
			" error, details: ", err.Error())
	}
	p.cfg.Network.BroadcastMessage(response)
}

func (p *ProposalDispatcher) OnResponseInactiveArbitratorsReceived(
	txHash *common.Uint256, signer []byte, sign []byte) {

	if !p.currentInactiveArbitratorTx.Hash().IsEqual(*txHash) {
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
	buf.Write(sign)
	pro.Parameter = buf.Bytes()

	p.tryEnterEmergencyState(len(pro.Parameter) / crypto.SignatureLength)
}

func (p *ProposalDispatcher) tryEnterEmergencyState(signCount int) bool {
	minSignCount := int(float64(p.cfg.Arbitrators.GetArbitersCount()) * 0.5)
	if signCount > minSignCount {
		p.illegalMonitor.AddEvidence(p.currentInactiveArbitratorTx.
			Payload.(*payload.InactiveArbitrators))
		p.cfg.Manager.AppendToTxnPool(p.currentInactiveArbitratorTx)

		blockchain.DefaultLedger.Blockchain.GetState().
			ProcessSpecialTxPayload(p.currentInactiveArbitratorTx.Payload)
		if err := p.cfg.Arbitrators.ForceChange(
			blockchain.DefaultLedger.Blockchain.GetHeight()); err != nil {
			log.Error("[tryEnterEmergencyState] force change arbitrators"+
				" error: ", err.Error())
			return false
		}

		p.illegalMonitor.SetInactiveArbitratorsTxHash(p.
			currentInactiveArbitratorTx.Hash())
		// we should clear existing blocks because they do not have inactive
		// arbitrators tx
		p.cfg.Manager.GetBlockCache().Reset()

		p.inactiveCountDown.SetEliminated()
		return true
	}

	return false
}

func (p *ProposalDispatcher) alreadyExistVote(v payload.DPOSProposalVote) bool {
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

func (p *ProposalDispatcher) countAcceptedVote(v payload.DPOSProposalVote) (succeed bool, finished bool) {
	log.Info("[countAcceptedVote] start")
	defer log.Info("[countAcceptedVote] end")

	if v.Accept {
		log.Info("[countAcceptedVote] Received needed sign, collect it into AcceptVotes!")
		p.acceptVotes[v.Hash()] = v

		if p.cfg.Manager.GetArbitrators().HasArbitersMajorityCount(uint32(len(p.acceptVotes))) {
			log.Info("Collect majority signs, finish proposal.")
			return true, p.FinishProposal()
		}
		return true, false
	}

	return false, false
}

func (p *ProposalDispatcher) countRejectedVote(v payload.DPOSProposalVote) (succeed bool, finished bool) {
	log.Info("[countRejectedVote] start")
	defer log.Info("[countRejectedVote] end")

	if !v.Accept {
		log.Info("[countRejectedVote] Received invalid sign, collect it into RejectedVotes!")
		p.rejectedVotes[v.Hash()] = v

		if p.cfg.Manager.GetArbitrators().HasArbitersMinorityCount(uint32(len(p.rejectedVotes))) {
			p.CleanProposals(true)
			p.cfg.Consensus.ChangeView()
			return true, true
		}
		return true, false
	}

	return false, false
}

func (p *ProposalDispatcher) acceptProposal(d payload.DPOSProposal) {
	log.Info("[acceptProposal] start")
	defer log.Info("[acceptProposal] end")

	p.setProcessingProposal(d)
	vote := payload.DPOSProposalVote{ProposalHash: d.Hash(), Signer: p.cfg.Manager.GetPublicKey(), Accept: true}
	var err error
	vote.Sign, err = p.cfg.Account.SignVote(&vote)
	if err != nil {
		log.Error("[acceptProposal] sign failed")
		return
	}
	voteMsg := &dmsg.Vote{Command: dmsg.CmdAcceptVote, Vote: vote}
	p.ProcessVote(vote, true)

	p.proposalProcessFinished = true
	p.cfg.Network.BroadcastMessage(voteMsg)
	log.Info("[acceptProposal] send acc_vote msg:", dmsg.GetMessageHash(voteMsg).String())

	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(vote.Signer),
		ReceivedTime: time.Now(), Result: true, RawData: &vote}
	p.cfg.EventMonitor.OnVoteArrived(&voteEvent)
}

func (p *ProposalDispatcher) rejectProposal(d payload.DPOSProposal) {
	p.setProcessingProposal(d)

	vote := payload.DPOSProposalVote{ProposalHash: d.Hash(), Signer: p.cfg.Manager.GetPublicKey(), Accept: false}
	var err error
	vote.Sign, err = p.cfg.Account.SignVote(&vote)
	if err != nil {
		log.Error("[rejectProposal] sign failed")
		return
	}
	msg := &dmsg.Vote{Command: dmsg.CmdRejectVote, Vote: vote}
	log.Info("[rejectProposal] send rej_vote msg:", dmsg.GetMessageHash(msg))

	_, ok := p.cfg.Manager.GetBlockCache().TryGetValue(d.BlockHash)
	if !ok {
		log.Error("[rejectProposal] can't find block")
		return
	}
	p.ProcessVote(vote, false)
	p.cfg.Network.BroadcastMessage(msg)

	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(vote.Signer),
		ReceivedTime: time.Now(), Result: false, RawData: &vote}
	p.cfg.EventMonitor.OnVoteArrived(&voteEvent)
}

func (p *ProposalDispatcher) setProcessingProposal(d payload.DPOSProposal) {
	p.processingProposal = &d

	for _, v := range p.pendingVotes {
		if v.ProposalHash.IsEqual(d.Hash()) {
			p.ProcessVote(v, v.Accept)
		}
	}
	p.pendingVotes = make(map[common.Uint256]payload.DPOSProposalVote)
}

func (p *ProposalDispatcher) CreateInactiveArbitrators() (
	*types.Transaction, error) {
	var err error

	inactivePayload := &payload.InactiveArbitrators{
		Sponsor:     p.cfg.Manager.GetPublicKey(),
		Arbitrators: [][]byte{},
		BlockHeight: p.CurrentHeight(),
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

	con := contract.Contract{Prefix: contract.PrefixMultiSig}
	if con.Code, err = p.createArbitratorsRedeemScript(); err != nil {
		return nil, err
	}

	programHash := con.ToProgramHash()
	tx := &types.Transaction{
		Version: types.TransactionVersion(blockchain.DefaultLedger.
			HeightVersions.GetDefaultTxVersion(p.CurrentHeight())),
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
	tx.Programs = []*program.Program{
		{
			Code:      con.Code,
			Parameter: sign,
		},
	}

	p.currentInactiveArbitratorTx = tx
	return tx, nil
}

func (p *ProposalDispatcher) createArbitratorsRedeemScript() ([]byte, error) {

	var pks []*crypto.PublicKey
	for _, v := range p.cfg.Arbitrators.GetArbitrators() {
		pk, err := crypto.DecodePoint(v)
		if err != nil {
			return nil, err
		}
		pks = append(pks, pk)
	}

	arbitratorsCount := len(p.cfg.Arbitrators.GetArbitrators())
	minSignCount := int(float64(arbitratorsCount) * 0.5)
	return contract.CreateMultiSigRedeemScript(minSignCount+1, pks)
}

func NewDispatcherAndIllegalMonitor(cfg ProposalDispatcherConfig) (
	*ProposalDispatcher, *IllegalBehaviorMonitor) {
	p := &ProposalDispatcher{
		cfg:                cfg,
		processingBlock:    nil,
		processingProposal: nil,
		acceptVotes:        make(map[common.Uint256]payload.DPOSProposalVote),
		rejectedVotes:      make(map[common.Uint256]payload.DPOSProposalVote),
		pendingProposals:   make(map[common.Uint256]payload.DPOSProposal),
		pendingVotes:       make(map[common.Uint256]payload.DPOSProposalVote),
		eventAnalyzer: store.NewEventStoreAnalyzer(store.EventStoreAnalyzerConfig{
			InactiveEliminateCount: cfg.InactiveEliminateCount,
			Store:       cfg.Store,
			Arbitrators: cfg.Arbitrators,
		}),
	}
	p.inactiveCountDown = ViewChangesCountDown{
		dispatcher:                    p,
		consensus:                     cfg.Consensus,
		arbitrators:                   cfg.Arbitrators,
		timeoutRefactor:               0,
		inactiveArbitratorsEliminated: false,
	}
	p.inactiveCountDown.Reset()

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
