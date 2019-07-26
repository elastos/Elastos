package manager

import (
	"bytes"
	"sort"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/dtime"
	"github.com/elastos/Elastos.ELA/dpos/log"
	dp2p "github.com/elastos/Elastos.ELA/dpos/p2p"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	dpeer "github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/elanet"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	// maxRequestedBlocks is the maximum number of requested block
	// hashes to store in memory.
	maxRequestedBlocks = msg.MaxInvPerMsg
)

type DPOSNetworkConfig struct {
	ProposalDispatcher *ProposalDispatcher
	Store              store.IDposStore
	PublicKey          []byte
	AnnounceAddr       func()
}

type DPOSNetwork interface {
	Initialize(dnConfig DPOSNetworkConfig)

	Start()
	Stop() error

	SendMessageToPeer(id dpeer.PID, msg p2p.Message) error
	BroadcastMessage(msg p2p.Message)

	UpdatePeers(peers []dpeer.PID)
	GetActivePeers() []dp2p.Peer
	RecoverTimeout()
}

type StatusSyncEventListener interface {
	OnPing(id dpeer.PID, height uint32)
	OnPong(id dpeer.PID, height uint32)
	OnBlock(id dpeer.PID, block *types.Block)
	OnInv(id dpeer.PID, blockHash common.Uint256)
	OnGetBlock(id dpeer.PID, blockHash common.Uint256)
	OnGetBlocks(id dpeer.PID, startBlockHeight, endBlockHeight uint32)
	OnResponseBlocks(id dpeer.PID, blockConfirms []*types.DposBlock)
	OnRequestConsensus(id dpeer.PID, height uint32)
	OnResponseConsensus(id dpeer.PID, status *dmsg.ConsensusStatus)
	OnRequestProposal(id dpeer.PID, hash common.Uint256)
	OnIllegalProposalReceived(id dpeer.PID, proposals *payload.DPOSIllegalProposals)
	OnIllegalVotesReceived(id dpeer.PID, votes *payload.DPOSIllegalVotes)
}

type NetworkEventListener interface {
	StatusSyncEventListener

	OnProposalReceived(id dpeer.PID, p *payload.DPOSProposal)
	OnVoteAccepted(id dpeer.PID, p *payload.DPOSProposalVote)
	OnVoteRejected(id dpeer.PID, p *payload.DPOSProposalVote)

	OnChangeView()
	OnBadNetwork()
	OnRecover()
	OnRecoverTimeout()

	OnBlockReceived(b *types.Block, confirmed bool)
	OnConfirmReceived(p *payload.Confirm)
	OnIllegalBlocksTxReceived(i *payload.DPOSIllegalBlocks)
	OnSidechainIllegalEvidenceReceived(s *payload.SidechainIllegalData)
	OnInactiveArbitratorsReceived(id dpeer.PID, tx *types.Transaction)
	OnResponseInactiveArbitratorsReceived(txHash *common.Uint256,
		Signer []byte, Sign []byte)
	OnInactiveArbitratorsAccepted(p *payload.InactiveArbitrators)
}

type AbnormalRecovering interface {
	CollectConsensusStatus(height uint32, status *dmsg.ConsensusStatus) error
	RecoverFromConsensusStatus(status *dmsg.ConsensusStatus) error
}

type DPOSManagerConfig struct {
	PublicKey   []byte
	Arbitrators state.Arbitrators
	ChainParams *config.Params
	TimeSource  dtime.MedianTimeSource
	Server      elanet.Server
}

type DPOSManager struct {
	publicKey  []byte
	blockCache *ConsensusBlockCache

	handler        *DPOSHandlerSwitch
	network        DPOSNetwork
	dispatcher     *ProposalDispatcher
	consensus      *Consensus
	illegalMonitor *IllegalBehaviorMonitor

	arbitrators state.Arbitrators
	blockPool   *mempool.BlockPool
	txPool      *mempool.TxPool
	chainParams *config.Params
	timeSource  dtime.MedianTimeSource
	server      elanet.Server
	broadcast   func(p2p.Message)

	recoverStarted     bool
	notHandledProposal map[string]struct{}
	statusMap          map[uint32]map[string]*dmsg.ConsensusStatus

	requestedBlocks map[common.Uint256]struct{}
}

func (d *DPOSManager) AppendConfirm(confirm *payload.Confirm) (bool, bool, error) {
	return d.blockPool.AppendConfirm(confirm)
}

func (d *DPOSManager) AppendBlock(block *types.Block) {
	d.blockPool.AddToBlockMap(block)
}

func NewManager(cfg DPOSManagerConfig) *DPOSManager {
	m := &DPOSManager{
		publicKey:          cfg.PublicKey,
		blockCache:         &ConsensusBlockCache{},
		arbitrators:        cfg.Arbitrators,
		chainParams:        cfg.ChainParams,
		timeSource:         cfg.TimeSource,
		server:             cfg.Server,
		notHandledProposal: make(map[string]struct{}),
		statusMap:          make(map[uint32]map[string]*dmsg.ConsensusStatus),
		requestedBlocks:    make(map[common.Uint256]struct{}),
	}
	m.blockCache.Reset(nil)

	return m
}

func (d *DPOSManager) Initialize(handler *DPOSHandlerSwitch,
	dispatcher *ProposalDispatcher, consensus *Consensus, network DPOSNetwork,
	illegalMonitor *IllegalBehaviorMonitor, blockPool *mempool.BlockPool,
	txPool *mempool.TxPool, broadcast func(message p2p.Message)) {
	d.handler = handler
	d.dispatcher = dispatcher
	d.consensus = consensus
	d.network = network
	d.illegalMonitor = illegalMonitor
	d.blockCache.Listener = d.dispatcher
	d.blockPool = blockPool
	d.txPool = txPool
	d.broadcast = broadcast
}

func (d *DPOSManager) AppendToTxnPool(txn *types.Transaction) error {
	return d.txPool.AppendToTxPool(txn)
}

func (d *DPOSManager) Broadcast(msg p2p.Message) {
	go d.broadcast(msg)
}

func (d *DPOSManager) GetPublicKey() []byte {
	return d.publicKey
}

func (d *DPOSManager) GetBlockCache() *ConsensusBlockCache {
	return d.blockCache
}

func (d *DPOSManager) GetArbitrators() state.Arbitrators {
	return d.arbitrators
}

func (d *DPOSManager) isCurrentArbiter() bool {
	return d.arbitrators.IsArbitrator(d.publicKey)
}

func (d *DPOSManager) isCRCArbiter() bool {
	return d.arbitrators.IsCRCArbitrator(d.publicKey)
}

func (d *DPOSManager) ProcessHigherBlock(b *types.Block) {
	if !d.illegalMonitor.IsBlockValid(b) {
		log.Info("[ProcessHigherBlock] received block do not contains illegal evidence, block hash: ", b.Hash())
		return
	}

	if !d.consensus.IsOnDuty() {
		log.Info("[ProcessHigherBlock] broadcast inv and try start new consensus")
		d.network.BroadcastMessage(dmsg.NewInventory(b.Hash()))
	}

	if d.handler.TryStartNewConsensus(b) {
		d.notHandledProposal = make(map[string]struct{})
	}
}

func (d *DPOSManager) ConfirmBlock() {
	d.handler.FinishConsensus()
	d.notHandledProposal = make(map[string]struct{})
}

func (d *DPOSManager) ChangeConsensus(onDuty bool) {
	d.handler.SwitchTo(onDuty)
}

func (d *DPOSManager) OnProposalReceived(id dpeer.PID, p *payload.DPOSProposal) {
	log.Info("[OnProposalReceived] started")
	defer log.Info("[OnProposalReceived] end")
	if !d.isCurrentArbiter() {
		return
	}
	if !d.handler.ProcessProposal(id, p) {
		pubKey := common.BytesToHexString(id[:])
		d.notHandledProposal[pubKey] = struct{}{}
		count := len(d.notHandledProposal)

		if d.arbitrators.HasArbitersMinorityCount(count) {
			log.Info("[OnProposalReceived] has minority not handled" +
				" proposals, need recover")
			if d.recoverAbnormalState() {
				log.Info("[OnProposalReceived] recover start")
			} else {
				log.Error("[OnProposalReceived] has no active peers recover failed")
			}
		}
	}
}

func (d *DPOSManager) OnVoteAccepted(id dpeer.PID, p *payload.DPOSProposalVote) {
	log.Info("[OnVoteReceived] started")
	defer log.Info("[OnVoteReceived] end")
	if !d.isCurrentArbiter() {
		return
	}
	_, finished := d.handler.ProcessAcceptVote(id, p)
	if finished {
		d.changeHeight()
	}
}

func (d *DPOSManager) OnVoteRejected(id dpeer.PID, p *payload.DPOSProposalVote) {
	log.Info("[OnVoteRejected] started")
	defer log.Info("[OnVoteRejected] end")
	if !d.isCurrentArbiter() {
		return
	}
	d.handler.ProcessRejectVote(id, p)
}

func (d *DPOSManager) OnPing(id dpeer.PID, height uint32) {
	d.processHeartBeat(id, height)
}

func (d *DPOSManager) OnPong(id dpeer.PID, height uint32) {
	d.processHeartBeat(id, height)
}

func (d *DPOSManager) OnBlock(id dpeer.PID, block *types.Block) {
	if !d.isCurrentArbiter() {
		return
	}
	log.Debug("[OnBlock] received block:", block.Hash().String())
	hash := block.Hash()
	if _, ok := d.requestedBlocks[hash]; !ok {
		log.Warn("[OnBlock] received unrequested block")
		return
	}
	delete(d.requestedBlocks, hash)
	if block.Header.Height == blockchain.DefaultLedger.Blockchain.GetHeight()+1 {
		if _, _, err := d.blockPool.AppendDposBlock(&types.DposBlock{
			Block: block,
		}); err != nil {
			log.Error("[OnBlock] err: ", err.Error())
		}
	}
}

func (d *DPOSManager) OnInv(id dpeer.PID, blockHash common.Uint256) {
	if !d.isCurrentArbiter() {
		return
	}
	if d.isBlockExist(blockHash) {
		return
	}
	if _, ok := d.requestedBlocks[blockHash]; ok {
		return
	}

	log.Info("[ProcessInv] send getblock:", blockHash.String())
	d.limitMap(d.requestedBlocks, maxRequestedBlocks)
	d.requestedBlocks[blockHash] = struct{}{}
	d.network.SendMessageToPeer(id, dmsg.NewGetBlock(blockHash))
}

func (d *DPOSManager) isBlockExist(blockHash common.Uint256) bool {
	block, _ := d.getBlock(blockHash)
	return block != nil
}

func (d *DPOSManager) OnGetBlock(id dpeer.PID, blockHash common.Uint256) {
	if !d.isCurrentArbiter() {
		return
	}
	if block, err := d.getBlock(blockHash); err == nil {
		d.network.SendMessageToPeer(id, msg.NewBlock(block))
	}
}

func (d *DPOSManager) OnGetBlocks(id dpeer.PID, startBlockHeight, endBlockHeight uint32) {
	d.handler.ResponseGetBlocks(id, startBlockHeight, endBlockHeight)
}

func (d *DPOSManager) OnResponseBlocks(id dpeer.PID, blockConfirms []*types.DposBlock) {
	log.Info("[OnResponseBlocks] start")
	defer log.Info("[OnResponseBlocks] end")
	if !d.isCurrentArbiter() {
		return
	}
	if err := blockchain.DefaultLedger.AppendDposBlocks(blockConfirms); err != nil {
		log.Error("Response blocks error: ", err)
	}
}

func (d *DPOSManager) OnRequestConsensus(id dpeer.PID, height uint32) {
	if !d.isCurrentArbiter() {
		return
	}
	d.handler.HelpToRecoverAbnormal(id, height)
}

func (d *DPOSManager) OnResponseConsensus(id dpeer.PID, status *dmsg.ConsensusStatus) {
	if !d.isCurrentArbiter() {
		return
	}
	log.Info("[OnResponseConsensus] status:", *status)
	if !d.handler.isAbnormal || !d.recoverStarted {
		return
	}
	log.Info("[OnResponseConsensus] collect recover status")
	if _, ok := d.statusMap[status.ViewOffset]; !ok {
		d.statusMap[status.ViewOffset] = make(map[string]*dmsg.ConsensusStatus)
	}
	d.statusMap[status.ViewOffset][common.BytesToHexString(id[:])] = status
}

func (d *DPOSManager) OnBadNetwork() {
	log.Info("[OnBadNetwork] found network bad")
}

func (d *DPOSManager) OnRecover() {
	if !d.isCurrentArbiter() {
		return
	}
	d.changeHeight()
	d.recoverAbnormalState()
}

func (d *DPOSManager) OnRecoverTimeout() {
	if d.recoverStarted == true {
		if len(d.statusMap) != 0 {
			d.DoRecover()
		}
		d.recoverStarted = false
		d.statusMap = make(map[uint32]map[string]*dmsg.ConsensusStatus)
	}
}

func (d *DPOSManager) recoverAbnormalState() bool {
	if d.recoverStarted {
		return false
	}

	if arbiters := d.arbitrators.GetArbitrators(); len(arbiters) != 0 {
		if peers := d.network.GetActivePeers(); len(peers) == 0 {
			log.Error("[recoverAbnormalState] can not find active peer")
			return false
		}
		d.recoverStarted = true
		d.handler.RequestAbnormalRecovering()
		go func() {
			<-time.NewTicker(time.Second * 2).C
			d.network.RecoverTimeout()
		}()
		return true
	}
	return false
}

func (d *DPOSManager) DoRecover() {
	var maxCount int
	var maxCountMaxViewOffset uint32
	for k, v := range d.statusMap {
		if maxCount < len(v) {
			maxCount = len(v)
			maxCountMaxViewOffset = k
		} else if maxCount == len(v) && maxCountMaxViewOffset < k {
			maxCountMaxViewOffset = k
		}
	}
	var status *dmsg.ConsensusStatus
	startTimes := make([]int64, 0)
	for _, v := range d.statusMap[maxCountMaxViewOffset] {
		if status == nil {
			if v.ConsensusStatus == consensusReady {
				d.notHandledProposal = make(map[string]struct{})
				return
			}
			status = v
		}
		startTimes = append(startTimes, v.ViewStartTime.UnixNano())
	}
	sort.Slice(startTimes, func(i, j int) bool {
		return startTimes[i] < startTimes[j]
	})
	medianTime := medianOf(startTimes)
	status.ViewStartTime = dtime.Int64ToTime(medianTime)
	offset, offsetTime := d.calculateOffsetTime(status.ViewStartTime)
	status.ViewOffset += offset
	status.ViewStartTime = d.timeSource.AdjustedTime().Add(-offsetTime)
	log.Infof("[DoRecover] recover received %d status at "+
		"viewoffset:%d", len(startTimes), status.ViewOffset)
	d.handler.RecoverAbnormal(status)

	d.notHandledProposal = make(map[string]struct{})
}

func (d *DPOSManager) calculateOffsetTime(
	startTime time.Time) (uint32, time.Duration) {
	now := d.timeSource.AdjustedTime()
	duration := now.Sub(startTime)
	offset := duration / d.consensus.currentView.signTolerance
	offsetTime := duration % d.consensus.currentView.signTolerance

	return uint32(offset), offsetTime
}

func medianOf(nums []int64) int64 {
	l := len(nums)

	if l == 0 {
		return 0
	}

	if l%2 == 0 {
		return (nums[l/2] + nums[l/2-1]) / 2
	}

	return nums[l/2]
}

func (d *DPOSManager) OnChangeView() {
	if d.consensus.TryChangeView() {
		log.Info("[TryChangeView] succeed")
	}
}

func (d *DPOSManager) OnBlockReceived(b *types.Block, confirmed bool) {
	log.Info("[OnBlockReceived] start")
	defer log.Info("[OnBlockReceived] end")

	if confirmed {
		d.ConfirmBlock()
		d.changeHeight()
		d.dispatcher.illegalMonitor.CleanByBlock(b)
		log.Info("[OnBlockReceived] received confirmed block")
		return
	}
	if !d.isCurrentArbiter() {
		return
	}
	for _, tx := range b.Transactions {
		if tx.IsInactiveArbitrators() {
			p := tx.Payload.(*payload.InactiveArbitrators)
			if err := d.arbitrators.ProcessSpecialTxPayload(p,
				blockchain.DefaultLedger.Blockchain.GetHeight()); err != nil {
				log.Errorf("process special tx payload err: %s", err.Error())
				return
			}
			d.clearInactiveData(p)
		}
	}

	if blockchain.DefaultLedger.Blockchain.GetHeight() < b.Height { //new height block coming
		d.ProcessHigherBlock(b)
	} else {
		log.Warn("a.Leger.LastBlock.Height", blockchain.DefaultLedger.Blockchain.GetHeight(), "b.Height", b.Height)
	}
}

func (d *DPOSManager) OnConfirmReceived(p *payload.Confirm) {
	log.Info("[OnConfirmReceived] started, hash:", p.Proposal.BlockHash)
	defer log.Info("[OnConfirmReceived] end")
	if !d.isCurrentArbiter() {
		return
	}
	d.ConfirmBlock()
	d.changeHeight()
}

func (d *DPOSManager) OnIllegalProposalReceived(id dpeer.PID, proposals *payload.DPOSIllegalProposals) {
	if err := blockchain.CheckDPOSIllegalProposals(proposals); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.illegalMonitor.AddEvidence(proposals)
}

func (d *DPOSManager) OnIllegalVotesReceived(id dpeer.PID, votes *payload.DPOSIllegalVotes) {
	if err := blockchain.CheckDPOSIllegalVotes(votes); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.illegalMonitor.AddEvidence(votes)
}

func (d *DPOSManager) OnIllegalBlocksTxReceived(i *payload.DPOSIllegalBlocks) {
	if !d.isCurrentArbiter() {
		return
	}
	if err := blockchain.CheckDPOSIllegalBlocks(i); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.illegalMonitor.AddEvidence(i)
	d.dispatcher.OnIllegalBlocksTxReceived(i)
}

func (d *DPOSManager) OnSidechainIllegalEvidenceReceived(s *payload.SidechainIllegalData) {
	if err := blockchain.CheckSidechainIllegalEvidence(s); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.illegalMonitor.AddEvidence(s)
	d.illegalMonitor.SendSidechainIllegalEvidenceTransaction(s)
}

func (d *DPOSManager) OnInactiveArbitratorsAccepted(p *payload.InactiveArbitrators) {
	if !d.isCurrentArbiter() {
		return
	}
	d.arbitrators.ProcessSpecialTxPayload(p, blockchain.DefaultLedger.Blockchain.GetHeight())
	d.clearInactiveData(p)
}

func (d *DPOSManager) clearInactiveData(p *payload.InactiveArbitrators) {
	d.illegalMonitor.AddEvidence(p)
	d.illegalMonitor.SetInactiveArbitratorsTxHash(p.Hash())
	d.dispatcher.currentInactiveArbitratorTx = nil
	if d.dispatcher.inactiveCountDown.SetEliminated(p.Hash()) {
		d.dispatcher.eventAnalyzer.Clear()
	}

	var blocks []*types.Block
	for _, v := range d.blockCache.ConsensusBlocks {
		if d.illegalMonitor.IsBlockValid(v) {
			blocks = append(blocks, v)
		}
	}
	d.blockCache.Reset(nil)
	for _, b := range blocks {
		d.blockCache.AddValue(b.Hash(), b)
	}

	if d.arbitrators.IsInactiveMode() || d.arbitrators.IsUnderstaffedMode() {
		d.dispatcher.ResetByCurrentView()
	}

	log.Info("clearInactiveData finished:", len(d.blockCache.ConsensusBlocks))
}

func (d *DPOSManager) OnInactiveArbitratorsReceived(id dpeer.PID,
	tx *types.Transaction) {
	if !d.isCRCArbiter() {
		return
	}
	if err := blockchain.CheckInactiveArbitrators(tx); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.dispatcher.OnInactiveArbitratorsReceived(id, tx)
}

func (d *DPOSManager) OnResponseInactiveArbitratorsReceived(
	txHash *common.Uint256, signers []byte, signs []byte) {
	if !d.isCurrentArbiter() {
		return
	}
	if !d.isCRCArbiter() || !d.arbitrators.IsCRCArbitrator(signers) {
		return
	}
	d.dispatcher.OnResponseInactiveArbitratorsReceived(txHash, signers, signs)
}

func (d *DPOSManager) OnRequestProposal(id dpeer.PID, hash common.Uint256) {
	currentProposal := d.dispatcher.GetProcessingProposal()
	if currentProposal != nil {
		responseProposal := &dmsg.Proposal{Proposal: *currentProposal}
		d.network.SendMessageToPeer(id, responseProposal)
	}
}

func (d *DPOSManager) changeHeight() {
	d.changeOnDuty()
}

func (d *DPOSManager) changeOnDuty() {
	currentArbiter := d.arbitrators.GetNextOnDutyArbitrator(0)
	onDuty := bytes.Equal(d.publicKey, currentArbiter)

	if onDuty {
		log.Info("[onDutyArbitratorChanged] onduty")
	} else {
		log.Info("[onDutyArbitratorChanged] not onduty")
	}
	d.ChangeConsensus(onDuty)
}

func (d *DPOSManager) processHeartBeat(id dpeer.PID, height uint32) {
	if d.tryRequestBlocks(id, height) {
		log.Info("Found higher block, requesting it.")
	}
}

func (d *DPOSManager) tryRequestBlocks(id dpeer.PID, sourceHeight uint32) bool {
	height := blockchain.DefaultLedger.Blockchain.GetHeight()
	if sourceHeight > height {
		m := &dmsg.GetBlocks{
			StartBlockHeight: height + 1,
			EndBlockHeight:   sourceHeight}
		d.network.SendMessageToPeer(id, m)

		return true
	}
	return false
}

func (d *DPOSManager) getBlock(blockHash common.Uint256) (*types.Block, error) {
	block, have := d.blockPool.GetBlock(blockHash)
	if have {
		return block, nil
	}

	return blockchain.DefaultLedger.GetBlockWithHash(blockHash)
}

// limitMap is a helper function for maps that require a maximum limit by
// evicting a random transaction if adding a new value would cause it to
// overflow the maximum allowed.
func (d *DPOSManager) limitMap(m map[common.Uint256]struct{}, limit int) {
	if len(m)+1 > limit {
		// Remove a random entry from the map.  For most compilers, Go's
		// range statement iterates starting at a random item although
		// that is not 100% guaranteed by the spec.  The iteration order
		// is not important here because an adversary would have to be
		// able to pull off preimage attacks on the hashing function in
		// order to target eviction of specific entries anyways.
		for txHash := range m {
			delete(m, txHash)
			return
		}
	}
}
