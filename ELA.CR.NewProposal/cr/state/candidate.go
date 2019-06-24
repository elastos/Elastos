package state

import "github.com/elastos/Elastos.ELA/common"

// CandidateState defines states during a CR candidates lifetime
type CandidateState struct {
	// todo complete me
}

// Candidate defines information about CR candidates during the CR vote period
type Candidate struct {
	// todo add CRInfo

	state CandidateState

	votes common.Fixed64
}
