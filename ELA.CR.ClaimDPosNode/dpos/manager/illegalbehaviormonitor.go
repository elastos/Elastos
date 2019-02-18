package manager

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const WaitHeightTolerance = uint32(1)

type IllegalBehaviorMonitor struct {
	dispatcher      *ProposalDispatcher
	cachedProposals map[common.Uint256]*payload.DPOSProposal

	evidenceCache evidenceCache
	manager       *DPOSManager

	inactiveArbitratorsTxHash *common.Uint256
}

func (i *IllegalBehaviorMonitor) AddEvidence(evidence payload.DPOSIllegalData) {
	i.evidenceCache.AddEvidence(evidence)
}

func (i *IllegalBehaviorMonitor) SetInactiveArbitratorsTxHash(
	hash common.Uint256) {
	i.inactiveArbitratorsTxHash = &hash
}

func (i *IllegalBehaviorMonitor) IsBlockValid(block *types.Block) bool {
	if i.inactiveArbitratorsTxHash != nil {
		hasInactiveArbitratorsTx := false
		for _, tx := range block.Transactions {
			if tx.Hash().IsEqual(*i.inactiveArbitratorsTxHash) {
				hasInactiveArbitratorsTx = true
			}
		}

		if !hasInactiveArbitratorsTx {
			return false
		}
	}

	return i.evidenceCache.IsBlockValid(block)
}

func (i *IllegalBehaviorMonitor) AddProposal(proposal payload.DPOSProposal) {
	i.cachedProposals[proposal.Hash()] = &proposal
}

func (i *IllegalBehaviorMonitor) Reset(changeView bool) {
	i.cachedProposals = make(map[common.Uint256]*payload.DPOSProposal)
	for k, v := range i.dispatcher.pendingProposals {
		i.cachedProposals[k] = &v
	}

	if !changeView {
		if i.dispatcher.processingBlock != nil {
			i.evidenceCache.Reset(i.dispatcher.processingBlock)
		}

		i.inactiveArbitratorsTxHash = nil
	}
}

func (i *IllegalBehaviorMonitor) IsLegalProposal(p *payload.DPOSProposal) (*payload.DPOSProposal, bool) {
	if i.isProposalsIllegal(p, i.dispatcher.processingProposal) {
		return i.dispatcher.processingProposal, false
	}

	for _, pending := range i.dispatcher.pendingProposals {
		if i.isProposalsIllegal(p, &pending) {
			return &pending, false
		}
	}

	return nil, true
}

func (i *IllegalBehaviorMonitor) ProcessIllegalProposal(
	first, second *payload.DPOSProposal) {

	firstBlock, _ := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(
		first.BlockHash)
	firstBlockByte := new(bytes.Buffer)
	firstBlock.Header.Serialize(firstBlockByte)
	secondBlock, _ := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(
		second.BlockHash)
	secondBlockByte := new(bytes.Buffer)
	secondBlock.Header.Serialize(secondBlockByte)

	evidences := &payload.DPOSIllegalProposals{
		Evidence: payload.ProposalEvidence{
			Proposal:    *first,
			BlockHeader: firstBlockByte.Bytes(),
			BlockHeight: firstBlock.Height,
		},
		CompareEvidence: payload.ProposalEvidence{
			Proposal:    *second,
			BlockHeader: secondBlockByte.Bytes(),
			BlockHeight: secondBlock.Height,
		},
	}

	i.AddEvidence(evidences)
	i.sendIllegalProposalTransaction(evidences)

	m := &dmsg.IllegalProposals{Proposals: *evidences}
	i.dispatcher.cfg.Network.BroadcastMessage(m)
}

func (i *IllegalBehaviorMonitor) sendIllegalProposalTransaction(evidences *payload.DPOSIllegalProposals) {
	tx := &types.Transaction{
		Version: types.TransactionVersion(blockchain.DefaultLedger.
			HeightVersions.GetDefaultTxVersion(i.dispatcher.processingBlock.
			Height)),
		TxType:         types.IllegalProposalEvidence,
		PayloadVersion: payload.PayloadIllegalProposalVersion,
		Payload:        evidences,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}

	if code := i.manager.AppendToTxnPool(tx); code == errors.Success {
		i.manager.Broadcast(msg.NewTx(tx))
	}
}

func (i *IllegalBehaviorMonitor) SendSidechainIllegalEvidenceTransaction(
	evidence *payload.SidechainIllegalData) {
	tx := &types.Transaction{
		Version: types.TransactionVersion(blockchain.DefaultLedger.
			HeightVersions.GetDefaultTxVersion(i.dispatcher.processingBlock.
			Height)),
		TxType:         types.IllegalSidechainEvidence,
		PayloadVersion: payload.PayloadSidechainIllegalDataVersion,
		Payload:        evidence,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}

	if code := i.manager.AppendToTxnPool(tx); code == errors.Success {
		i.manager.Broadcast(msg.NewTx(tx))
	}
}

func (i *IllegalBehaviorMonitor) sendIllegalVoteTransaction(
	evidences *payload.DPOSIllegalVotes) {
	tx := &types.Transaction{
		Version:        types.TransactionVersion(blockchain.DefaultLedger.HeightVersions.GetDefaultTxVersion(i.dispatcher.processingBlock.Height)),
		TxType:         types.IllegalVoteEvidence,
		PayloadVersion: payload.PayloadIllegalVoteVersion,
		Payload:        evidences,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}

	if code := i.manager.AppendToTxnPool(tx); code == errors.Success {
		i.manager.Broadcast(msg.NewTx(tx))
	}
}

func (i *IllegalBehaviorMonitor) ProcessIllegalVote(
	first, second *payload.DPOSProposalVote) {

	firstProposal, _ := i.cachedProposals[first.ProposalHash]
	secondProposal, _ := i.cachedProposals[second.ProposalHash]

	firstBlock, _ := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(
		firstProposal.BlockHash)
	firstBlockByte := new(bytes.Buffer)
	firstBlock.Header.Serialize(firstBlockByte)
	secondBlock, _ := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(
		secondProposal.BlockHash)
	secondBlockByte := new(bytes.Buffer)
	secondBlock.Header.Serialize(secondBlockByte)

	evidences := &payload.DPOSIllegalVotes{
		Evidence: payload.VoteEvidence{
			Vote: *first,
			ProposalEvidence: payload.ProposalEvidence{
				Proposal:    *firstProposal,
				BlockHeader: firstBlockByte.Bytes(),
				BlockHeight: firstBlock.Height,
			},
		},
		CompareEvidence: payload.VoteEvidence{
			Vote: *second,
			ProposalEvidence: payload.ProposalEvidence{
				Proposal:    *secondProposal,
				BlockHeader: secondBlockByte.Bytes(),
				BlockHeight: secondBlock.Height,
			},
		},
	}

	i.AddEvidence(evidences)
	i.sendIllegalVoteTransaction(evidences)

	m := &dmsg.IllegalVotes{Votes: *evidences}
	i.dispatcher.cfg.Network.BroadcastMessage(m)
}

func (i *IllegalBehaviorMonitor) isProposalsIllegal(first, second *payload.DPOSProposal) bool {
	if first == nil || second == nil {
		return false
	}

	if !bytes.Equal(first.Sponsor, second.Sponsor) || first.ViewOffset != second.ViewOffset {
		return false
	}

	firstBlock, foundFirst := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(first.BlockHash)
	secondBlock, foundSecond := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(second.BlockHash)
	if !foundFirst || !foundSecond {
		return false
	}

	if firstBlock.Height == secondBlock.Height {
		return true
	}

	return false
}

func (i *IllegalBehaviorMonitor) IsLegalVote(v *payload.DPOSProposalVote) (*payload.DPOSProposalVote, bool) {
	for _, accept := range i.dispatcher.acceptVotes {
		if i.isVotesIllegal(v, &accept) {
			return &accept, false
		}
	}

	for _, reject := range i.dispatcher.rejectedVotes {
		if i.isVotesIllegal(v, &reject) {
			return &reject, false
		}
	}

	return nil, true
}

func (i *IllegalBehaviorMonitor) isVotesIllegal(first, second *payload.DPOSProposalVote) bool {
	if !bytes.Equal(first.Signer, second.Signer) {
		return false
	}

	if first.ProposalHash.IsEqual(second.ProposalHash) {
		return !first.Hash().IsEqual(second.Hash())
	}

	firstProposal, foundFirst := i.cachedProposals[first.ProposalHash]
	secondProposal, foundSecond := i.cachedProposals[second.ProposalHash]
	if !foundFirst || !foundSecond {
		return false
	}

	return i.isProposalsIllegal(firstProposal, secondProposal)
}
