package manager

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	dmsg "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const WaitHeightTolerance = uint32(1)

type IllegalBehaviorMonitor struct {
	dispatcher      *ProposalDispatcher
	cachedProposals map[common.Uint256]*payload.DPOSProposal

	evidenceCache evidenceCache
	manager       *DPOSManager

	inactiveArbitratorsPayloadHash *common.Uint256
}

func (i *IllegalBehaviorMonitor) AddEvidence(evidence payload.DPOSIllegalData) {
	i.evidenceCache.AddEvidence(evidence)
}

func (i *IllegalBehaviorMonitor) SetInactiveArbitratorsTxHash(
	hash common.Uint256) {
	i.inactiveArbitratorsPayloadHash = &hash
}

func (i *IllegalBehaviorMonitor) IsBlockValid(block *types.Block) bool {
	if i.inactiveArbitratorsPayloadHash != nil {
		hasInactiveArbitratorsTx := false
		for _, tx := range block.Transactions {
			if tx.IsInactiveArbitrators() &&
				tx.Payload.(*payload.InactiveArbitrators).Hash().IsEqual(*i.inactiveArbitratorsPayloadHash) {
				hasInactiveArbitratorsTx = true
			}
		}

		if !hasInactiveArbitratorsTx {
			return false
		}
	}

	return i.evidenceCache.IsBlockValid(block)
}

func (i *IllegalBehaviorMonitor) AddProposal(proposal *payload.DPOSProposal) {
	i.cachedProposals[proposal.Hash()] = proposal
}

func (i *IllegalBehaviorMonitor) Reset(changeView bool) {
	i.cachedProposals = make(map[common.Uint256]*payload.DPOSProposal)

	if !changeView {
		if i.dispatcher.processingBlock != nil {
			i.evidenceCache.Reset(i.dispatcher.processingBlock)
		}

		i.inactiveArbitratorsPayloadHash = nil
	} else {
		for k, v := range i.dispatcher.pendingProposals {
			i.cachedProposals[k] = v
		}
		for k, v := range i.dispatcher.precociousProposals {
			i.cachedProposals[k] = v
		}
	}
}

func (i *IllegalBehaviorMonitor) CleanByBlock(b *types.Block) {
	for _, tx := range b.Transactions {
		if tx.IsIllegalTypeTx() || tx.IsInactiveArbitrators() {
			hash := tx.Payload.(payload.DPOSIllegalData).Hash()
			i.evidenceCache.TryDelete(hash)

			if tx.IsIllegalProposalTx() {
				delete(i.cachedProposals, hash)
			}
		}
	}
}

func (i *IllegalBehaviorMonitor) IsLegalProposal(p *payload.DPOSProposal) (*payload.DPOSProposal, bool) {
	if i.isProposalsIllegal(p, i.dispatcher.processingProposal) {
		return i.dispatcher.processingProposal, false
	}

	for _, pending := range i.dispatcher.pendingProposals {
		if i.isProposalsIllegal(p, pending) {
			return pending, false
		}
	}
	for _, pre := range i.dispatcher.precociousProposals {
		if i.isProposalsIllegal(p, pre) {
			return pre, false
		}
	}

	return nil, true
}

func (i *IllegalBehaviorMonitor) generateProposalEvidence(
	p *payload.DPOSProposal) (*payload.ProposalEvidence, error) {

	block, _ := i.dispatcher.cfg.Manager.GetBlockCache().TryGetValue(
		p.BlockHash)
	blockByte := new(bytes.Buffer)
	if err := block.Header.Serialize(blockByte); err != nil {
		return nil, err
	}

	return &payload.ProposalEvidence{
		Proposal:    *p,
		BlockHeader: blockByte.Bytes(),
		BlockHeight: block.Height,
	}, nil
}

func (i *IllegalBehaviorMonitor) ProcessIllegalProposal(
	first, second *payload.DPOSProposal) {

	firstEvidence, err := i.generateProposalEvidence(first)
	if err != nil {
		log.Warn("[ProcessIllegalProposal] generate evidence error: ", err)
	}

	secondEvidence, err := i.generateProposalEvidence(second)
	if err != nil {
		log.Warn("[ProcessIllegalProposal] generate evidence error: ", err)
	}

	asc := true
	if first.Hash().Compare(second.Hash()) > 0 {
		asc = false
	}

	evidences := &payload.DPOSIllegalProposals{}
	if asc {
		evidences.Evidence = *firstEvidence
		evidences.CompareEvidence = *secondEvidence
	} else {
		evidences.Evidence = *secondEvidence
		evidences.CompareEvidence = *firstEvidence
	}

	i.AddEvidence(evidences)
	i.sendIllegalProposalTransaction(evidences)

	m := &dmsg.IllegalProposals{Proposals: *evidences}
	i.dispatcher.cfg.Network.BroadcastMessage(m)
}

func (i *IllegalBehaviorMonitor) sendIllegalProposalTransaction(
	evidences *payload.DPOSIllegalProposals) {
	tx := &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.IllegalProposalEvidence,
		PayloadVersion: payload.IllegalProposalVersion,
		Payload:        evidences,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}

	if err := i.manager.AppendToTxnPool(tx); err == nil {
		i.manager.Broadcast(msg.NewTx(tx))
	}
}

func (i *IllegalBehaviorMonitor) SendSidechainIllegalEvidenceTransaction(
	evidence *payload.SidechainIllegalData) {
	tx := &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.IllegalSidechainEvidence,
		PayloadVersion: payload.SidechainIllegalDataVersion,
		Payload:        evidence,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}

	if err := i.manager.AppendToTxnPool(tx); err == nil {
		i.manager.Broadcast(msg.NewTx(tx))
	}
}

func (i *IllegalBehaviorMonitor) sendIllegalVoteTransaction(
	evidences *payload.DPOSIllegalVotes) {
	tx := &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.IllegalVoteEvidence,
		PayloadVersion: payload.IllegalVoteVersion,
		Payload:        evidences,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}

	if err := i.manager.AppendToTxnPool(tx); err == nil {
		i.manager.Broadcast(msg.NewTx(tx))
	}
}

func (i *IllegalBehaviorMonitor) ProcessIllegalVote(
	first, second *payload.DPOSProposalVote) {

	firstProposal, ok := i.cachedProposals[first.ProposalHash]
	if !ok {
		log.Warn("[ProcessIllegalVote] found proposal error")
		return
	}

	secondProposal, ok := i.cachedProposals[second.ProposalHash]
	if !ok {
		log.Warn("[ProcessIllegalVote] found proposal error")
		return
	}

	firstEvidence, err := i.generateProposalEvidence(firstProposal)
	if err != nil {
		log.Warn("[ProcessIllegalProposal] generate evidence error: ", err)
		return
	}

	secondEvidence, err := i.generateProposalEvidence(secondProposal)
	if err != nil {
		log.Warn("[ProcessIllegalProposal] generate evidence error: ", err)
		return
	}

	asc := true
	if first.Hash().Compare(second.Hash()) > 0 {
		asc = false
	}

	evidences := &payload.DPOSIllegalVotes{}
	if asc {
		evidences.Evidence = payload.VoteEvidence{
			Vote:             *first,
			ProposalEvidence: *firstEvidence,
		}
		evidences.CompareEvidence = payload.VoteEvidence{
			Vote:             *second,
			ProposalEvidence: *secondEvidence,
		}
	} else {
		evidences.Evidence = payload.VoteEvidence{
			Vote:             *second,
			ProposalEvidence: *secondEvidence,
		}
		evidences.CompareEvidence = payload.VoteEvidence{
			Vote:             *first,
			ProposalEvidence: *firstEvidence,
		}
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

	if first.BlockHash.IsEqual(second.BlockHash) {
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
		if i.isVotesIllegal(v, accept) {
			return accept, false
		}
	}

	for _, reject := range i.dispatcher.rejectedVotes {
		if i.isVotesIllegal(v, reject) {
			return reject, false
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
