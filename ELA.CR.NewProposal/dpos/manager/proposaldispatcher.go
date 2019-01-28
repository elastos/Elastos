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
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type ProposalDispatcher interface {
	AbnormalRecovering

	//status
	GetProcessingBlock() *types.Block
	GetProcessingProposal() *types.DPosProposal
	IsProcessingBlockEmpty() bool
	CurrentHeight() uint32

	//proposal
	StartProposal(b *types.Block)
	CleanProposals(changeView bool)
	FinishProposal()
	TryStartSpeculatingProposal(b *types.Block)
	ProcessProposal(d types.DPosProposal)

	FinishConsensus()

	ProcessVote(v types.DPosProposalVote, accept bool)
	AddPendingVote(v types.DPosProposalVote)

	//abnormal
	OnAbnormalStateDetected()
	RequestAbnormalRecovering()
	TryAppendAndBroadcastConfirmBlockMsg() bool

	//inactive arbitrators
	OnInactiveArbitratorsReceived(tx *types.Transaction)
	OnResponseInactiveArbitratorsReceived(txHash *common.Uint256, signer []byte,
		sign []byte)
	CreateInactiveArbitrators() (*types.Transaction, error)
	HasEnteredEmergency() bool
}

type ProposalDispatcherConfig struct {
	store.EventStoreAnalyzerConfig
	EventMonitor *log.EventMonitor
	Consensus    Consensus
	Network      DposNetwork
	Manager      DposManager
	Account      account.DposAccount
	ChainParams  *config.Params
}

type proposalDispatcher struct {
	cfg ProposalDispatcherConfig

	processingBlock    *types.Block
	processingProposal *types.DPosProposal
	acceptVotes        map[common.Uint256]types.DPosProposalVote
	rejectedVotes      map[common.Uint256]types.DPosProposalVote
	pendingProposals   map[common.Uint256]types.DPosProposal
	pendingVotes       map[common.Uint256]types.DPosProposalVote

	processingInactiveArbitratorTx *types.Transaction

	eventAnalyzer  *store.EventStoreAnalyzer
	illegalMonitor IllegalBehaviorMonitor
}

func (p *proposalDispatcher) OnAbnormalStateDetected() {
	p.RequestAbnormalRecovering()
}

func (p *proposalDispatcher) RequestAbnormalRecovering() {
	height := p.CurrentHeight()
	msgItem := &dmsg.RequestConsensus{Height: height}
	peerID := p.cfg.Network.GetActivePeer()
	if peerID == nil {
		log.Error("[RequestAbnormalRecovering] can not find active peer")
		return
	}
	p.cfg.Network.SendMessageToPeer(*peerID, msgItem)
}

func (p *proposalDispatcher) GetProcessingBlock() *types.Block {
	return p.processingBlock
}

func (p *proposalDispatcher) GetProcessingProposal() *types.DPosProposal {
	return p.processingProposal
}

func (p *proposalDispatcher) ProcessVote(v types.DPosProposalVote, accept bool) {
	log.Info("[ProcessVote] start")
	defer log.Info("[ProcessVote] end")

	if !blockchain.IsVoteValid(&v) {
		log.Info("Invalid vote")
		return
	}

	if p.alreadyExistVote(v) {
		log.Info("Already has vote")
		return
	}

	if anotherVote, legal := p.illegalMonitor.IsLegalVote(&v); !legal {
		p.illegalMonitor.ProcessIllegalVote(&v, anotherVote)
		return
	}

	if accept {
		p.countAcceptedVote(v)
	} else {
		p.countRejectedVote(v)
	}
}

func (p *proposalDispatcher) AddPendingVote(v types.DPosProposalVote) {
	p.pendingVotes[v.Hash()] = v
}

func (p *proposalDispatcher) IsProcessingBlockEmpty() bool {
	return p.processingBlock == nil
}

func (p *proposalDispatcher) StartProposal(b *types.Block) {
	log.Info("[StartProposal] start")
	defer log.Info("[StartProposal] end")

	if p.processingBlock != nil {
		log.Info("[StartProposal] start proposal failed")
		return
	}
	p.processingBlock = b

	p.cfg.Network.BroadcastMessage(dmsg.NewInventory(b.Hash()))
	proposal := types.DPosProposal{Sponsor: p.cfg.Manager.GetPublicKey(), BlockHash: b.Hash(), ViewOffset: p.cfg.Consensus.GetViewOffset()}
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

	rawData := new(bytes.Buffer)
	proposal.Serialize(rawData)
	proposalEvent := log.ProposalEvent{
		Proposal:     common.BytesToHexString(proposal.Sponsor),
		BlockHash:    proposal.BlockHash,
		ReceivedTime: time.Now(),
		ProposalHash: proposal.Hash(),
		RawData:      rawData.Bytes(),
		Result:       false,}
	p.cfg.EventMonitor.OnProposalArrived(&proposalEvent)
	p.acceptProposal(proposal)
}

func (p *proposalDispatcher) TryStartSpeculatingProposal(b *types.Block) {
	log.Info("[TryStartSpeculatingProposal] start")
	defer log.Info("[TryStartSpeculatingProposal] end")

	if p.processingBlock != nil {
		log.Warn("[TryStartSpeculatingProposal] processingBlock is not nil")
		return
	}
	p.processingBlock = b
}

func (p *proposalDispatcher) FinishProposal() {
	log.Info("[FinishProposal] start")
	defer log.Info("[FinishProposal] end")

	if p.processingBlock == nil {
		log.Warn("[FinishProposal] nil processing block")
		return
	}

	proposal, blockHash := p.processingProposal.Sponsor, p.processingBlock.Hash()

	if !p.TryAppendAndBroadcastConfirmBlockMsg() {
		log.Warn("Add block failed, no need to broadcast confirm message")
		return
	}

	p.FinishConsensus()

	proposalEvent := log.ProposalEvent{
		Proposal:  common.BytesToHexString(proposal),
		BlockHash: blockHash,
		EndTime:   time.Now(),
		Result:    true,
	}
	p.cfg.EventMonitor.OnProposalFinished(&proposalEvent)
}

func (p *proposalDispatcher) CleanProposals(changeView bool) {
	log.Info("Clean proposals")

	//todo clear pending proposals that are lower than current consensus height
	p.illegalMonitor.Reset(changeView)

	p.processingBlock = nil
	p.processingProposal = nil
	p.acceptVotes = make(map[common.Uint256]types.DPosProposalVote)
	p.rejectedVotes = make(map[common.Uint256]types.DPosProposalVote)
	p.pendingVotes = make(map[common.Uint256]types.DPosProposalVote)
}

func (p *proposalDispatcher) ProcessProposal(d types.DPosProposal) {
	log.Info("[ProcessProposal] start")
	defer log.Info("[ProcessProposal] end")

	if p.HasEnteredEmergency() {
		log.Info("enter emergency state, proposal will be discard")
		return
	}

	if p.processingProposal != nil && d.BlockHash.IsEqual(p.processingProposal.Hash()) {
		log.Info("already processing processing")
		return
	}

	if _, ok := p.pendingProposals[d.Hash()]; ok {
		log.Info("already have proposal, wait for processing")
		return
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

	p.acceptProposal(d)
}

func (p *proposalDispatcher) TryAppendAndBroadcastConfirmBlockMsg() bool {
	currentVoteSlot := &types.DPosProposalVoteSlot{
		Hash:     p.processingBlock.Hash(),
		Proposal: *p.processingProposal,
		Votes:    make([]types.DPosProposalVote, 0),
	}
	for _, v := range p.acceptVotes {
		currentVoteSlot.Votes = append(currentVoteSlot.Votes, v)
	}

	log.Info("[TryAppendAndBroadcastConfirmBlockMsg] append confirm.")
	p.cfg.Manager.Broadcast(msg.NewBlock(&types.DposBlock{
		ConfirmFlag: true,
		Confirm:     currentVoteSlot,
	}))
	inMainChain, isOrphan, err := p.cfg.Manager.AppendConfirm(currentVoteSlot)
	if err != nil || !inMainChain || isOrphan {
		log.Error("[AppendConfirm] err:", err.Error())
		return false
	}

	return true
}

func (p *proposalDispatcher) OnBlockAdded(b *types.Block) {

	if p.cfg.Consensus.IsRunning() {
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
	if p.cfg.Consensus.IsRunning() {
		log.Info("[FinishConsensus] start")
		defer log.Info("[FinishConsensus] end")

		c := log.ConsensusEvent{EndTime: time.Now(), Height: p.CurrentHeight()}
		p.cfg.EventMonitor.OnConsensusFinished(&c)
		p.cfg.Consensus.SetReady()
		p.CleanProposals(false)
	}
}

func (p *proposalDispatcher) CollectConsensusStatus(height uint32, status *dmsg.ConsensusStatus) error {
	if height > p.CurrentHeight() {
		return errors.New("Requesting height greater than current processing height")
	}

	status.AcceptVotes = make([]types.DPosProposalVote, 0, len(p.acceptVotes))
	for _, v := range p.acceptVotes {
		status.AcceptVotes = append(status.AcceptVotes, v)
	}

	status.RejectedVotes = make([]types.DPosProposalVote, 0, len(p.rejectedVotes))
	for _, v := range p.rejectedVotes {
		status.RejectedVotes = append(status.RejectedVotes, v)
	}

	status.PendingProposals = make([]types.DPosProposal, 0, len(p.pendingProposals))
	for _, v := range p.pendingProposals {
		status.PendingProposals = append(status.PendingProposals, v)
	}

	status.PendingVotes = make([]types.DPosProposalVote, 0, len(p.pendingVotes))
	for _, v := range p.pendingVotes {
		status.PendingVotes = append(status.PendingVotes, v)
	}

	return nil
}

func (p *proposalDispatcher) RecoverFromConsensusStatus(status *dmsg.ConsensusStatus) error {
	p.acceptVotes = make(map[common.Uint256]types.DPosProposalVote)
	for _, v := range status.AcceptVotes {
		p.acceptVotes[v.Hash()] = v
	}

	p.rejectedVotes = make(map[common.Uint256]types.DPosProposalVote)
	for _, v := range status.RejectedVotes {
		p.rejectedVotes[v.Hash()] = v
	}

	p.pendingProposals = make(map[common.Uint256]types.DPosProposal)
	for _, v := range status.PendingProposals {
		p.pendingProposals[v.Hash()] = v
	}

	p.pendingVotes = make(map[common.Uint256]types.DPosProposalVote)
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
		height = blockchain.DefaultLedger.Blockchain.GetHeight()
	}
	return height
}

func (p *proposalDispatcher) HasEnteredEmergency() bool {
	return p.CurrentHeight() > config.Parameters.HeightVersions[3] &&
		p.cfg.Consensus.GetViewOffset() >= p.cfg.Arbitrators.GetArbitersCount()
}

func (p *proposalDispatcher) OnInactiveArbitratorsReceived(
	tx *types.Transaction) {
	var err error

	if !p.HasEnteredEmergency() {
		log.Warn("[OnInactiveArbitratorsReceived] received inactive" +
			" arbitrators transaction when normal view changing")
		return
	}

	if err = blockchain.CheckInactiveArbitrators(tx,
		p.cfg.ChainParams.InactiveEliminateCount); err != nil {
		log.Warn("[OnInactiveArbitratorsReceived] check tx error, details: ",
			err.Error())
		return
	}

	inactivePayload := tx.Payload.(*payload.InactiveArbitrators)
	if !p.cfg.Consensus.IsArbitratorOnDuty(inactivePayload.Sponsor) {
		log.Warn("[OnInactiveArbitratorsReceived] sender is not on duty")
		return
	}

	if !p.processingInactiveArbitratorTx.Hash().IsEqual(tx.Hash()) {
		p.processingInactiveArbitratorTx = tx
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

func (p *proposalDispatcher) OnResponseInactiveArbitratorsReceived(
	txHash *common.Uint256, signer []byte, sign []byte) {

	if !p.processingInactiveArbitratorTx.Hash().IsEqual(*txHash) {
		log.Warn("[OnResponseInactiveArbitratorsReceived] unknown " +
			"inactive arbitrators transaction")
		return
	}

	data := new(bytes.Buffer)
	if err := p.processingInactiveArbitratorTx.SerializeUnsigned(
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

	pro := p.processingInactiveArbitratorTx.Programs[0]
	buf := new(bytes.Buffer)
	buf.Write(pro.Parameter)
	buf.Write(sign)
	pro.Parameter = buf.Bytes()

	minSignCount := int(float64(p.cfg.Arbitrators.GetArbitersCount()) * 0.5)
	if len(pro.Parameter)/crypto.SignatureLength > minSignCount {
		p.cfg.Manager.AppendToTxnPool(p.processingInactiveArbitratorTx)
	}
}

func (p *proposalDispatcher) alreadyExistVote(v types.DPosProposalVote) bool {
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

func (p *proposalDispatcher) countAcceptedVote(v types.DPosProposalVote) {
	log.Info("[countAcceptedVote] start")
	defer log.Info("[countAcceptedVote] end")

	if v.Accept {
		log.Info("[countAcceptedVote] Received needed sign, collect it into AcceptVotes!")
		p.acceptVotes[v.Hash()] = v

		if p.cfg.Manager.GetArbitrators().HasArbitersMajorityCount(uint32(len(p.acceptVotes))) {
			log.Info("Collect majority signs, finish proposal.")
			p.FinishProposal()
		}
	}
}

func (p *proposalDispatcher) countRejectedVote(v types.DPosProposalVote) {
	log.Info("[countRejectedVote] start")
	defer log.Info("[countRejectedVote] end")

	if !v.Accept {
		log.Info("[countRejectedVote] Received invalid sign, collect it into RejectedVotes!")
		p.rejectedVotes[v.Hash()] = v

		if p.cfg.Manager.GetArbitrators().HasArbitersMinorityCount(uint32(len(p.rejectedVotes))) {
			p.CleanProposals(true)
			p.cfg.Consensus.ChangeView()
		}
	}
}

func (p *proposalDispatcher) acceptProposal(d types.DPosProposal) {
	log.Info("[acceptProposal] start")
	defer log.Info("[acceptProposal] end")

	p.setProcessingProposal(d)
	vote := types.DPosProposalVote{ProposalHash: d.Hash(), Signer: p.cfg.Manager.GetPublicKey(), Accept: true}
	var err error
	vote.Sign, err = p.cfg.Account.SignVote(&vote)
	if err != nil {
		log.Error("[acceptProposal] sign failed")
		return
	}
	voteMsg := &dmsg.Vote{Command: dmsg.CmdAcceptVote, Vote: vote}
	p.ProcessVote(vote, true)

	p.cfg.Network.BroadcastMessage(voteMsg)
	log.Info("[acceptProposal] send acc_vote msg:", dmsg.GetMessageHash(voteMsg).String())

	rawData := new(bytes.Buffer)
	vote.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(vote.Signer), ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	p.cfg.EventMonitor.OnVoteArrived(&voteEvent)
}

func (p *proposalDispatcher) rejectProposal(d types.DPosProposal) {
	p.setProcessingProposal(d)

	vote := types.DPosProposalVote{ProposalHash: d.Hash(), Signer: p.cfg.Manager.GetPublicKey(), Accept: false}
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

	rawData := new(bytes.Buffer)
	vote.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(vote.Signer), ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	p.cfg.EventMonitor.OnVoteArrived(&voteEvent)
}

func (p *proposalDispatcher) setProcessingProposal(d types.DPosProposal) {
	p.processingProposal = &d

	for _, v := range p.pendingVotes {
		if v.ProposalHash.IsEqual(d.Hash()) {
			p.ProcessVote(v, v.Accept)
		}
	}
	p.pendingVotes = make(map[common.Uint256]types.DPosProposalVote)
}

func (p *proposalDispatcher) CreateInactiveArbitrators() (
	*types.Transaction, error) {
	var err error

	inactivePayload := &payload.InactiveArbitrators{
		Sponsor: p.cfg.Manager.GetPublicKey(), Arbitrators: [][]byte{}}
	inactiveArbitrators := p.eventAnalyzer.ParseInactiveArbitrators()
	for _, v := range inactiveArbitrators {
		var pk []byte
		pk, err = common.HexStringToBytes(v)
		if err != nil {
			return nil, err
		}
		inactivePayload.Arbitrators = append(inactivePayload.Arbitrators, pk)
	}

	con := contract.Contract{HashPrefix: contract.PrefixMultiSig}
	if con.Code, err = p.createArbitratorsRedeemScript(); err != nil {
		return nil, err
	}

	var programHash *common.Uint168
	if programHash, err = con.ToProgramHash(); err != nil {
		return nil, err
	}

	tx := &types.Transaction{
		Version:        types.TransactionVersion(blockchain.DefaultLedger.HeightVersions.GetDefaultTxVersion(p.processingBlock.Height)),
		TxType:         types.InactiveArbitrators,
		PayloadVersion: payload.PayloadInactiveArbitratorsVersion,
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

	p.processingInactiveArbitratorTx = tx
	return tx, nil
}

func (p *proposalDispatcher) createArbitratorsRedeemScript() ([]byte, error) {

	var pks []*crypto.PublicKey
	for _, v := range p.cfg.Arbitrators.GetArbitrators() {
		pk, err := crypto.DecodePoint(v)
		if err != nil {
			return nil, err
		}
		pks = append(pks, pk)
	}

	arbitratorsCount := len(p.cfg.Arbitrators.GetArbitrators())
	minSignCount := uint(float64(arbitratorsCount) * 0.5)
	return crypto.CreateMultiSignRedeemScript(minSignCount+1, pks)
}

func NewDispatcherAndIllegalMonitor(cfg ProposalDispatcherConfig) (ProposalDispatcher, IllegalBehaviorMonitor) {
	p := &proposalDispatcher{
		cfg:                cfg,
		processingBlock:    nil,
		processingProposal: nil,
		acceptVotes:        make(map[common.Uint256]types.DPosProposalVote),
		rejectedVotes:      make(map[common.Uint256]types.DPosProposalVote),
		pendingProposals:   make(map[common.Uint256]types.DPosProposal),
		pendingVotes:       make(map[common.Uint256]types.DPosProposalVote),
		eventAnalyzer: store.NewEventStoreAnalyzer(store.EventStoreAnalyzerConfig{
			InactivePercentage: cfg.InactivePercentage,
			Store:              cfg.Store,
			Arbitrators:        cfg.Arbitrators.(*store.Arbitrators),
		}),
	}
	i := &illegalBehaviorMonitor{
		dispatcher:      p,
		cachedProposals: make(map[common.Uint256]*types.DPosProposal),
		evidenceCache:   evidenceCache{make(map[common.Uint256]types.DposIllegalData)},
		manager:         cfg.Manager,
	}
	p.illegalMonitor = i
	return p, i
}
