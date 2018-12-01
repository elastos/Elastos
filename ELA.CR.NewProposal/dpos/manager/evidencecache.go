package manager

import (
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type evidenceCache struct {
	evidences map[common.Uint256]types.DposIllegalData
}

func (e *evidenceCache) AddEvidence(evidence types.DposIllegalData) {
	if evidence != nil {
		e.evidences[evidence.Hash()] = evidence
	}
}

func (e *evidenceCache) IsBlockValid(block *types.Block) bool {
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

func (e *evidenceCache) Reset(block *types.Block) {
	for _, t := range block.Transactions {
		if hash, ok := e.tryGetEvidenceHash(t); ok {
			if _, hasEvidence := e.evidences[hash]; hasEvidence {
				delete(e.evidences, hash)
			}
		}
	}
}

func (e *evidenceCache) tryGetEvidenceHash(tx *types.Transaction) (common.Uint256, bool) {
	var hash common.Uint256
	result := false

	switch tx.TxType {
	case types.IllegalProposalEvidence:
		proposalPayload := tx.Payload.(*types.PayloadIllegalProposal)
		hash = proposalPayload.DposIllegalProposals.Hash()
		result = true
	case types.IllegalVoteEvidence:
		votePayload := tx.Payload.(*types.PayloadIllegalVote)
		hash = votePayload.DposIllegalVotes.Hash()
		result = true
	case types.IllegalBlockEvidence:
		blockPayload := tx.Payload.(*types.PayloadIllegalBlock)
		hash = blockPayload.DposIllegalBlocks.Hash()
		result = true
	}

	return hash, result
}
