package manager

import (
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type evidenceCache struct {
	evidences map[common.Uint256]core.DposIllegalData
}

func (e *evidenceCache) AddEvidence(evidence core.DposIllegalData) {
	if evidence != nil {
		e.evidences[evidence.Hash()] = evidence
	}
}

func (e *evidenceCache) IsBlockValid(block *core.Block) bool {
	necessaryEvidences := make(map[common.Uint256]interface{})
	for k, v := range e.evidences {
		if v.GetBlockHeight()+WaitHeightTolerance <= block.Height {
			necessaryEvidences[k] = nil
		}
	}

	for _, t := range block.Transactions {
		if hash, ok := e.tryGetEvidenceHash(t); ok {
			if _, hasEvidence := necessaryEvidences[hash]; hasEvidence {
				delete(necessaryEvidences, hash)
			}
		}
	}

	return len(necessaryEvidences) == 0
}

func (e *evidenceCache) Reset(block *core.Block) {
	for _, t := range block.Transactions {
		if hash, ok := e.tryGetEvidenceHash(t); ok {
			if _, hasEvidence := e.evidences[hash]; hasEvidence {
				delete(e.evidences, hash)
			}
		}
	}
}

func (e *evidenceCache) tryGetEvidenceHash(tx *core.Transaction) (common.Uint256, bool) {
	var hash common.Uint256
	result := false

	switch tx.TxType {
	case core.IllegalProposalEvidence:
		proposalPayload := tx.Payload.(*core.PayloadIllegalProposal)
		hash = proposalPayload.DposIllegalProposals.Hash()
		result = true
	case core.IllegalVoteEvidence:
		votePayload := tx.Payload.(*core.PayloadIllegalVote)
		hash = votePayload.DposIllegalVotes.Hash()
		result = true
	case core.IllegalBlockEvidence:
		blockPayload := tx.Payload.(*core.PayloadIllegalBlock)
		hash = blockPayload.DposIllegalBlocks.Hash()
		result = true
	}

	return hash, result
}
