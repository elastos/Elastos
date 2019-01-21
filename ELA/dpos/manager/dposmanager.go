package manager

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/log"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	dpeer "github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type DposNetworkConfig struct {
	ProposalDispatcher ProposalDispatcher
	Store              interfaces.IDposStore
}

type DposNetwork interface {
	Initialize(dnConfig DposNetworkConfig)

	Start()
	Stop() error

	SendMessageToPeer(id dpeer.PID, msg p2p.Message) error
	BroadcastMessage(msg p2p.Message)

	UpdatePeers(arbitrators [][]byte) error
	ChangeHeight(height uint32) error

	GetActivePeer() *dpeer.PID
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
	OnIllegalProposalReceived(id dpeer.PID, proposals *types.DposIllegalProposals)
	OnIllegalVotesReceived(id dpeer.PID, votes *types.DposIllegalVotes)
}

type NetworkEventListener interface {
	StatusSyncEventListener

	OnProposalReceived(id dpeer.PID, p types.DPosProposal)
	OnVoteReceived(id dpeer.PID, p types.DPosProposalVote)
	OnVoteRejected(id dpeer.PID, p types.DPosProposalVote)

	OnChangeView()
	OnBadNetwork()

	OnBlockReceived(b *types.Block, confirmed bool)
	OnConfirmReceived(p *types.DPosProposalVoteSlot)
	OnIllegalBlocksReceived(i *types.DposIllegalBlocks)
	OnSidechainIllegalEvidenceReceived(s *types.SidechainIllegalData)
	OnInactiveArbitratorsReceived(tx *types.Transaction)
	OnResponseInactiveArbitratorsReceived(tx *types.Transaction, Signer []byte, Sign []byte)
}

type AbnormalRecovering interface {
	CollectConsensusStatus(height uint32, status *dmsg.ConsensusStatus) error
	RecoverFromConsensusStatus(status *dmsg.ConsensusStatus) error
}

type DposManager interface {
	NetworkEventListener

	GetPublicKey() []byte
	GetBlockCache() *ConsensusBlockCache
	GetArbitrators() interfaces.Arbitrators

	Initialize(handler DposHandlerSwitch, dispatcher ProposalDispatcher,
		consensus Consensus, network DposNetwork,
		illegalMonitor IllegalBehaviorMonitor, blockPool *mempool.BlockPool,
		txPool *mempool.TxPool, broadcast func(message p2p.Message))

	Recover()

	ProcessHigherBlock(b *types.Block)
	ConfirmBlock()

	ChangeConsensus(onDuty bool)

	AppendConfirm(confirm *types.DPosProposalVoteSlot) (bool, bool, error)
	AppendToTxnPool(txn *types.Transaction) errors.ErrCode
	Broadcast(msg p2p.Message)
}

type DposManagerConfig struct {
	PublicKey   []byte
	Arbitrators interfaces.Arbitrators
}

type dposManager struct {
	publicKey  []byte
	blockCache *ConsensusBlockCache

	handler        DposHandlerSwitch
	network        DposNetwork
	dispatcher     ProposalDispatcher
	consensus      Consensus
	illegalMonitor IllegalBehaviorMonitor

	arbitrators interfaces.Arbitrators
	blockPool   *mempool.BlockPool
	txPool      *mempool.TxPool
	broadcast   func(p2p.Message)
}

func (d *dposManager) AppendConfirm(confirm *types.DPosProposalVoteSlot) (bool, bool, error) {
	return d.blockPool.AppendConfirm(confirm)
}

func NewManager(cfg DposManagerConfig) DposManager {
	m := &dposManager{
		publicKey:   cfg.PublicKey,
		blockCache:  &ConsensusBlockCache{},
		arbitrators: cfg.Arbitrators,
	}
	m.blockCache.Reset()

	return m
}

func (d *dposManager) Initialize(handler DposHandlerSwitch,
	dispatcher ProposalDispatcher, consensus Consensus, network DposNetwork,
	illegalMonitor IllegalBehaviorMonitor, blockPool *mempool.BlockPool,
	txPool *mempool.TxPool, broadcast func(message p2p.Message)) {
	d.handler = handler
	d.dispatcher = dispatcher
	d.consensus = consensus
	d.network = network
	d.illegalMonitor = illegalMonitor
	d.blockCache.Listener = d.dispatcher.(*proposalDispatcher)
	d.blockPool = blockPool
	d.txPool = txPool
	d.broadcast = broadcast
}

func (d *dposManager) AppendToTxnPool(txn *types.Transaction) errors.ErrCode {
	return d.txPool.AppendToTxnPool(txn)
}

func (d *dposManager) Broadcast(msg p2p.Message) {
	d.broadcast(msg)
}

func (d *dposManager) GetPublicKey() []byte {
	return d.publicKey
}

func (d *dposManager) GetBlockCache() *ConsensusBlockCache {
	return d.blockCache
}

func (d *dposManager) GetArbitrators() interfaces.Arbitrators {
	return d.arbitrators
}

func (d *dposManager) Recover() {
	d.changeHeight()
	for {
		if peerID := d.network.GetActivePeer(); peerID != nil {
			d.handler.RequestAbnormalRecovering()
			return
		}

		time.Sleep(time.Second)
	}
}

func (d *dposManager) ProcessHigherBlock(b *types.Block) {
	if !d.illegalMonitor.IsBlockValid(b) {
		log.Info("[ProcessHigherBlock] received block do not contains illegal evidence, block hash: ", b.Hash())
		return
	}

	log.Info("[ProcessHigherBlock] broadcast inv and try start new consensus")
	d.network.BroadcastMessage(dmsg.NewInventory(b.Hash()))
	d.handler.TryStartNewConsensus(b)
}

func (d *dposManager) ConfirmBlock() {
	d.handler.FinishConsensus()
}

func (d *dposManager) ChangeConsensus(onDuty bool) {
	d.handler.SwitchTo(onDuty)
}

func (d *dposManager) OnProposalReceived(id dpeer.PID, p types.DPosProposal) {
	log.Info("[OnProposalReceived] started")
	defer log.Info("[OnProposalReceived] end")

	d.handler.StartNewProposal(p)
}

func (d *dposManager) OnVoteReceived(id dpeer.PID, p types.DPosProposalVote) {
	log.Info("[OnVoteReceived] started")
	defer log.Info("[OnVoteReceived] end")
	d.handler.ProcessAcceptVote(id, p)
}

func (d *dposManager) OnVoteRejected(id dpeer.PID, p types.DPosProposalVote) {
	log.Info("[OnVoteRejected] started")
	defer log.Info("[OnVoteRejected] end")
	d.handler.ProcessRejectVote(id, p)
}

func (d *dposManager) OnPing(id dpeer.PID, height uint32) {
	d.processHeartBeat(id, height)
}

func (d *dposManager) OnPong(id dpeer.PID, height uint32) {
	d.processHeartBeat(id, height)
}

func (d *dposManager) OnBlock(id dpeer.PID, block *types.Block) {
	log.Info("[ProcessBlock] received block:", block.Hash().String())
	if block.Header.Height == blockchain.DefaultLedger.Blockchain.GetHeight()+1 {
		if _, _, err := d.blockPool.AppendDposBlock(&types.DposBlock{
			BlockFlag: true,
			Block:     block,
		}); err != nil {
			log.Error("[OnBlock] err: ", err.Error())
		}
	}
}

func (d *dposManager) OnInv(id dpeer.PID, blockHash common.Uint256) {
	if _, err := d.getBlock(blockHash); err != nil {
		log.Info("[ProcessInv] send getblock:", blockHash.String())
		d.network.SendMessageToPeer(id, dmsg.NewGetBlock(blockHash))
	}
}

func (d *dposManager) OnGetBlock(id dpeer.PID, blockHash common.Uint256) {
	if block, err := d.getBlock(blockHash); err == nil {
		d.network.SendMessageToPeer(id, msg.NewBlock(block))
	}
}

func (d *dposManager) OnGetBlocks(id dpeer.PID, startBlockHeight, endBlockHeight uint32) {
	d.handler.ResponseGetBlocks(id, startBlockHeight, endBlockHeight)
}

func (d *dposManager) OnResponseBlocks(id dpeer.PID, blockConfirms []*types.DposBlock) {
	log.Info("[OnResponseBlocks] start")
	defer log.Info("[OnResponseBlocks] end")

	if err := blockchain.DefaultLedger.AppendDposBlocks(blockConfirms); err != nil {
		log.Error("Response blocks error: ", err)
	}
}

func (d *dposManager) OnRequestConsensus(id dpeer.PID, height uint32) {
	d.handler.HelpToRecoverAbnormal(id, height)
}

func (d *dposManager) OnResponseConsensus(id dpeer.PID, status *dmsg.ConsensusStatus) {
	log.Info("[OnResponseConsensus], status:", *status)
	d.handler.RecoverAbnormal(status)
}

func (d *dposManager) OnBadNetwork() {
	d.dispatcher.OnAbnormalStateDetected()
}

func (d *dposManager) OnChangeView() {
	d.consensus.TryChangeView()
}

func (d *dposManager) OnBlockReceived(b *types.Block, confirmed bool) {
	log.Info("[OnBlockReceived] start")
	defer log.Info("[OnBlockReceived] end")

	if confirmed {
		d.ConfirmBlock()
		d.changeHeight()
		log.Info("[OnBlockReceived] received confirmed block")
		return
	}

	if blockchain.DefaultLedger.Blockchain.GetHeight() < b.Height { //new height block coming
		d.ProcessHigherBlock(b)
	} else {
		log.Warn("a.Leger.LastBlock.Height", blockchain.DefaultLedger.Blockchain.GetHeight(), "b.Height", b.Height)
	}
}

func (d *dposManager) OnConfirmReceived(p *types.DPosProposalVoteSlot) {

	log.Info("[OnConfirmReceived] started, hash:", p.Hash)
	defer log.Info("[OnConfirmReceived] end")

	d.ConfirmBlock()
	d.changeHeight()
}

func (d *dposManager) OnIllegalProposalReceived(id dpeer.PID, proposals *types.DposIllegalProposals) {
	d.illegalMonitor.AddEvidence(proposals)
}

func (d *dposManager) OnIllegalVotesReceived(id dpeer.PID, votes *types.DposIllegalVotes) {
	d.illegalMonitor.AddEvidence(votes)
}

func (d *dposManager) OnIllegalBlocksReceived(i *types.DposIllegalBlocks) {
	d.illegalMonitor.AddEvidence(i)
}

func (d *dposManager) OnSidechainIllegalEvidenceReceived(s *types.SidechainIllegalData) {
	d.illegalMonitor.AddEvidence(s)
	d.illegalMonitor.SendSidechainIllegalEvidenceTransaction(s)
}

func (d *dposManager) OnInactiveArbitratorsReceived(tx *types.Transaction) {
	d.dispatcher.OnInactiveArbitratorsReceived(tx)
}

func (d *dposManager) OnResponseInactiveArbitratorsReceived(tx *types.Transaction, signers []byte, signs []byte) {
	d.dispatcher.OnResponseInactiveArbitratorsReceived(tx, signers, signs)
}

func (d *dposManager) OnRequestProposal(id dpeer.PID, hash common.Uint256) {
	currentProposal := d.dispatcher.GetProcessingProposal()
	if currentProposal != nil {
		responseProposal := &dmsg.Proposal{Proposal: *currentProposal}
		d.network.SendMessageToPeer(id, responseProposal)
	}
}

func (d *dposManager) changeHeight() {
	if err := d.network.ChangeHeight(d.dispatcher.CurrentHeight()); err != nil {
		log.Error("Error occurred with change height: ", err)
		return
	}

	currentArbiter := d.arbitrators.GetOnDutyArbitrator()
	onDuty := bytes.Equal(d.publicKey, currentArbiter)

	if onDuty {
		log.Info("[onDutyArbitratorChanged] not onduty -> onduty")
	} else {
		log.Info("[onDutyArbitratorChanged] onduty -> not onduty")
	}
	d.ChangeConsensus(onDuty)
}

func (d *dposManager) processHeartBeat(id dpeer.PID, height uint32) {
	if d.tryRequestBlocks(id, height) {
		log.Info("Found higher block, requesting it.")
	}
}

func (d *dposManager) tryRequestBlocks(id dpeer.PID, sourceHeight uint32) bool {
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

func (d *dposManager) getBlock(blockHash common.Uint256) (*types.Block, error) {
	block, have := d.blockPool.GetBlock(blockHash)
	if have {
		return block, nil
	}

	return blockchain.DefaultLedger.GetBlockWithHash(blockHash)
}
