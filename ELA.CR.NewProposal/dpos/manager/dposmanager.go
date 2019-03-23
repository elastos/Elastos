package manager

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	dp2p "github.com/elastos/Elastos.ELA/dpos/p2p"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	dpeer "github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/dpos/store"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type DPOSNetworkConfig struct {
	ProposalDispatcher *ProposalDispatcher
	Store              store.IDposStore
	PublicKey          []byte
}

type DPOSNetwork interface {
	Initialize(dnConfig DPOSNetworkConfig)

	Start()
	Stop() error

	SendMessageToPeer(id dpeer.PID, msg p2p.Message) error
	BroadcastMessage(msg p2p.Message)

	UpdatePeers(arbitrators map[string]*dp2p.PeerAddr) error
	GetActivePeers() []dp2p.Peer
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

	OnBlockReceived(b *types.Block, confirmed bool)
	OnConfirmReceived(p *payload.Confirm)
	OnIllegalBlocksTxReceived(i *payload.DPOSIllegalBlocks)
	OnSidechainIllegalEvidenceReceived(s *payload.SidechainIllegalData)
	OnInactiveArbitratorsReceived(tx *types.Transaction)
	OnResponseInactiveArbitratorsReceived(txHash *common.Uint256,
		Signer []byte, Sign []byte)
}

type AbnormalRecovering interface {
	CollectConsensusStatus(height uint32, status *dmsg.ConsensusStatus) error
	RecoverFromConsensusStatus(status *dmsg.ConsensusStatus) error
}

type DPOSManagerConfig struct {
	PublicKey   []byte
	Arbitrators state.Arbitrators
	ChainParams *config.Params
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
	broadcast   func(p2p.Message)

	notHandledProposal   map[string]struct{}
	neededMajorityStatus int
	statusMap            map[uint32]map[string]*dmsg.ConsensusStatus
}

func (d *DPOSManager) AppendConfirm(confirm *payload.Confirm) (bool, bool, error) {
	return d.blockPool.AppendConfirm(confirm)
}

func (d *DPOSManager) AppendBlock(block *types.Block) {
	d.blockPool.AddToBlockMap(block)
}

func NewManager(cfg DPOSManagerConfig) *DPOSManager {
	m := &DPOSManager{
		publicKey:   cfg.PublicKey,
		blockCache:  &ConsensusBlockCache{},
		arbitrators: cfg.Arbitrators,
		chainParams: cfg.ChainParams,
	}
	m.blockCache.Reset()

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
	d.statusMap = make(map[uint32]map[string]*dmsg.ConsensusStatus)
}

func (d *DPOSManager) AppendToTxnPool(txn *types.Transaction) errors.ErrCode {
	return d.txPool.AppendToTxnPool(txn)
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

func (d *DPOSManager) Recover() {
	d.changeHeight()
	for {
		log.Info("Recover when start")
		if d.recoverAbnormalState() {
			log.Info("Recover finished")
			return
		}

		time.Sleep(time.Second)
	}
}

func (d *DPOSManager) ProcessHigherBlock(b *types.Block) {
	if !d.illegalMonitor.IsBlockValid(b) {
		log.Info("[ProcessHigherBlock] received block do not contains illegal evidence, block hash: ", b.Hash())
		return
	}

	//log.Info("[ProcessHigherBlock] broadcast inv and try start new consensus")
	//d.network.BroadcastMessage(dmsg.NewInventory(b.Hash()))

	if d.handler.TryStartNewConsensus(b) {
		d.notHandledProposal = make(map[string]struct{})
		d.statusMap = make(map[uint32]map[string]*dmsg.ConsensusStatus)
	}
}

func (d *DPOSManager) ConfirmBlock() {
	d.handler.FinishConsensus()
}

func (d *DPOSManager) ChangeConsensus(onDuty bool) {
	d.handler.SwitchTo(onDuty)
}

func (d *DPOSManager) OnProposalReceived(id dpeer.PID, p *payload.DPOSProposal) {
	log.Info("[OnProposalReceived] started")
	defer log.Info("[OnProposalReceived] end")

	if !d.handler.ProcessProposal(id, p) {
		pubKey := common.BytesToHexString(id[:])
		d.notHandledProposal[pubKey] = struct{}{}
		if d.arbitrators.HasArbitersMinorityCount(len(d.notHandledProposal)) {
			log.Info("[OnVoteAccepted] has minority not handled votes, need recover")
			if d.recoverAbnormalState() {
				log.Info("[OnVoteAccepted] recover start")
			}
			log.Error("[OnVoteAccepted] has no active peers recover failed")
		}
	}
}

func (d *DPOSManager) OnVoteAccepted(id dpeer.PID, p *payload.DPOSProposalVote) {
	log.Info("[OnVoteReceived] started")
	defer log.Info("[OnVoteReceived] end")
	_, finished := d.handler.ProcessAcceptVote(id, p)
	if finished {
		d.changeHeight()
	}
}

func (d *DPOSManager) OnVoteRejected(id dpeer.PID, p *payload.DPOSProposalVote) {
	log.Info("[OnVoteRejected] started")
	defer log.Info("[OnVoteRejected] end")
	d.handler.ProcessRejectVote(id, p)
}

func (d *DPOSManager) OnPing(id dpeer.PID, height uint32) {
	d.processHeartBeat(id, height)
}

func (d *DPOSManager) OnPong(id dpeer.PID, height uint32) {
	d.processHeartBeat(id, height)
}

func (d *DPOSManager) OnBlock(id dpeer.PID, block *types.Block) {
	log.Info("[ProcessBlock] received block:", block.Hash().String())
	if block.Header.Height == blockchain.DefaultLedger.Blockchain.GetHeight()+1 {
		if _, _, err := d.blockPool.AppendDposBlock(&types.DposBlock{
			Block: block,
		}); err != nil {
			log.Error("[OnBlock] err: ", err.Error())
		}
	}
}

func (d *DPOSManager) OnInv(id dpeer.PID, blockHash common.Uint256) {
	if _, err := d.getBlock(blockHash); err != nil {
		log.Info("[ProcessInv] send getblock:", blockHash.String())
		d.network.SendMessageToPeer(id, dmsg.NewGetBlock(blockHash))
	}
}

func (d *DPOSManager) OnGetBlock(id dpeer.PID, blockHash common.Uint256) {
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

	if err := blockchain.DefaultLedger.AppendDposBlocks(blockConfirms); err != nil {
		log.Error("Response blocks error: ", err)
	}
}

func (d *DPOSManager) OnRequestConsensus(id dpeer.PID, height uint32) {
	d.handler.HelpToRecoverAbnormal(id, height)
}

func (d *DPOSManager) OnResponseConsensus(id dpeer.PID, status *dmsg.ConsensusStatus) {
	log.Info("[OnResponseConsensus] status:", *status)
	if !d.handler.isAbnormal {
		return
	}
	log.Info("[OnResponseConsensus] collect recover status")
	if _, ok := d.statusMap[status.ViewOffset]; !ok {
		d.statusMap[status.ViewOffset] = make(map[string]*dmsg.ConsensusStatus)
	}
	d.statusMap[status.ViewOffset][common.BytesToHexString(id[:])] = status
	if len(d.statusMap[status.ViewOffset]) >= d.neededMajorityStatus {
		log.Infof("[OnResponseConsensus] recover received %d status at "+
			"viewoffset:%d", len(d.statusMap[status.ViewOffset]), status.ViewOffset)
		d.handler.RecoverAbnormal(status)
		d.notHandledProposal = make(map[string]struct{})
		d.statusMap = make(map[uint32]map[string]*dmsg.ConsensusStatus)
	}
}

func (d *DPOSManager) OnBadNetwork() {
	log.Info("[OnBadNetwork] found network bad")
	if d.recoverAbnormalState() {
		log.Info("[OnBadNetwork] start recover")
	}
	log.Error("[OnBadNetwork] has no active peers recover failed")
}

func (d *DPOSManager) recoverAbnormalState() bool {
	if arbiters := d.arbitrators.GetArbitrators(); len(arbiters) != 0 {
		if peers := d.network.GetActivePeers(); len(peers) == 0 {
			log.Error("[recoverAbnormalState] can not find active peer")
			return false
		}

		d.neededMajorityStatus = len(arbiters)/3 + 1
		d.handler.RequestAbnormalRecovering()
		return true
	}
	return false
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
		log.Info("[OnBlockReceived] received confirmed block")
		return
	}

	isCurrentArbiter := false
	arbiters := d.arbitrators.GetArbitrators()
	for _, a := range arbiters {
		if bytes.Equal(a, d.publicKey) {
			isCurrentArbiter = true
		}
	}
	if !isCurrentArbiter {
		return
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
	if err := blockchain.CheckDPOSIllegalBlocks(i); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.illegalMonitor.AddEvidence(i)
}

func (d *DPOSManager) OnSidechainIllegalEvidenceReceived(s *payload.SidechainIllegalData) {
	if err := blockchain.CheckSidechainIllegalEvidence(s); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.illegalMonitor.AddEvidence(s)
	d.illegalMonitor.SendSidechainIllegalEvidenceTransaction(s)
}

func (d *DPOSManager) OnInactiveArbitratorsReceived(tx *types.Transaction) {
	if err := blockchain.CheckInactiveArbitrators(tx,
		d.chainParams.InactiveEliminateCount); err != nil {
		log.Info("[OnIllegalProposalReceived] received error evidence: ", err)
		return
	}
	d.dispatcher.OnInactiveArbitratorsReceived(tx)
}

func (d *DPOSManager) OnResponseInactiveArbitratorsReceived(
	txHash *common.Uint256, signers []byte, signs []byte) {
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
	height := d.dispatcher.CurrentHeight()
	if sourceHeight > height {
		m := &dmsg.GetBlocks{
			StartBlockHeight: height,
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
