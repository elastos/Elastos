// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"math/rand"
	"testing"

	"github.com/stretchr/testify/suite"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type confirmValidatorTestSuite struct {
	suite.Suite

	originalLedger *Ledger
	arbitrators    *state.ArbitratorsMock
}

func init() {
	testing.Init()
}

func (s *confirmValidatorTestSuite) SetupSuite() {
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}

	s.arbitrators = &state.ArbitratorsMock{
		CurrentArbitrators: make([]state.ArbiterMember, 0),
		MajorityCount:      3,
	}
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		ar, _ := state.NewOriginArbiter(state.Origin, a)
		s.arbitrators.CurrentArbitrators = append(
			s.arbitrators.CurrentArbitrators, ar)
		s.arbitrators.ActiveProducer = append(s.arbitrators.ActiveProducer, ar)
	}

	s.originalLedger = DefaultLedger
	DefaultLedger = &Ledger{Arbitrators: s.arbitrators}
}

func (s *confirmValidatorTestSuite) TearDownSuite() {
	DefaultLedger = s.originalLedger
}

func (s *confirmValidatorTestSuite) TestProposalSanityCheck() {
	pri, pk, _ := crypto.GenerateKeyPair()
	pkBuf, _ := pk.EncodePoint(true)

	proposal := &payload.DPOSProposal{
		Sponsor:    pkBuf,
		ViewOffset: rand.Uint32(),
		BlockHash:  *randomUint256(),
		Sign:       randomSignature(),
	}

	s.Error(ProposalSanityCheck(proposal), "signature verify error")

	proposal.Sign, _ = crypto.Sign(pri, proposal.Data())
	s.NoError(ProposalSanityCheck(proposal))
}

func (s *confirmValidatorTestSuite) TestProposalContextCheck() {
	proposal := &payload.DPOSProposal{
		Sponsor:    randomPublicKey(),
		ViewOffset: rand.Uint32(),
		BlockHash:  *randomUint256(),
		Sign:       randomSignature(),
	}

	s.EqualError(ProposalContextCheck(proposal),
		"current arbitrators verify error")

	for _, v := range s.arbitrators.CurrentArbitrators {
		proposal.Sponsor = v.GetNodePublicKey()
		s.NoError(ProposalContextCheck(proposal))
	}
}

func (s *confirmValidatorTestSuite) TestVoteSanityCheck() {
	pri, pk, _ := crypto.GenerateKeyPair()
	pkBuf, _ := pk.EncodePoint(true)

	vote := &payload.DPOSProposalVote{
		ProposalHash: *randomUint256(),
		Signer:       pkBuf,
		Accept:       true,
		Sign:         randomSignature(),
	}

	s.Error(VoteSanityCheck(vote), "signature verify error")

	vote.Sign, _ = crypto.Sign(pri, vote.Data())
	s.NoError(VoteSanityCheck(vote))
}

func (s *confirmValidatorTestSuite) TestVoteContextCheck() {
	vote := &payload.DPOSProposalVote{
		ProposalHash: *randomUint256(),
		Signer:       randomPublicKey(),
		Accept:       true,
		Sign:         randomSignature(),
	}

	s.EqualError(VoteContextCheck(vote),
		"current arbitrators verify error")

	for _, v := range s.arbitrators.CurrentArbitrators {
		vote.Signer = v.GetNodePublicKey()
		s.NoError(VoteContextCheck(vote))
	}
}

func (s *confirmValidatorTestSuite) TestConfirmSanityCheck() {
	pri, pk, _ := crypto.GenerateKeyPair()
	pkBuf, _ := pk.EncodePoint(true)

	pri2, pk2, _ := crypto.GenerateKeyPair()
	pkBuf2, _ := pk2.EncodePoint(true)

	confirm := &payload.Confirm{
		Proposal: payload.DPOSProposal{
			Sponsor:    pkBuf,
			ViewOffset: rand.Uint32(),
			BlockHash:  *randomUint256(),
			Sign:       randomSignature(),
		},
		Votes: []payload.DPOSProposalVote{
			{
				ProposalHash: *randomUint256(),
				Signer:       pkBuf,
				Accept:       false,
				Sign:         randomSignature(),
			},
			{
				ProposalHash: *randomUint256(),
				Signer:       pkBuf2,
				Accept:       false,
				Sign:         randomSignature(),
			},
		},
	}

	s.Error(ConfirmSanityCheck(confirm), "proposal sanity error")

	confirm.Proposal.Sign, _ = crypto.Sign(pri, confirm.Proposal.Data())
	s.EqualError(ConfirmSanityCheck(confirm),
		"[ConfirmSanityCheck] confirm contains reject vote")

	vote := confirm.Votes[0]
	vote.Accept = true
	confirm.Votes[0] = vote
	vote = confirm.Votes[1]
	vote.Accept = true
	confirm.Votes[1] = vote
	s.EqualError(ConfirmSanityCheck(confirm),
		"[ConfirmSanityCheck] confirm contains invalid vote")

	confirm.Votes[0].ProposalHash = confirm.Proposal.Hash()
	confirm.Votes[0].Sign, _ = crypto.Sign(pri, confirm.Votes[0].Data())
	s.EqualError(ConfirmSanityCheck(confirm),
		"[ConfirmSanityCheck] confirm contains invalid vote",
		"second vote sanity error")

	confirm.Votes[1].ProposalHash = confirm.Proposal.Hash()
	confirm.Votes[1].Sign, _ = crypto.Sign(pri2, confirm.Votes[1].Data())

	s.NoError(ConfirmSanityCheck(confirm))
}

func (s *confirmValidatorTestSuite) TestConfirmContextCheck() {
	confirm := &payload.Confirm{
		Proposal: payload.DPOSProposal{
			Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			ViewOffset: rand.Uint32(),
			BlockHash:  *randomUint256(),
			Sign:       randomSignature(),
		},
		Votes: []payload.DPOSProposalVote{},
	}

	s.EqualError(ConfirmContextCheck(confirm),
		"[ConfirmContextCheck] signers less than majority count")

	// repeat votes about on duty signer
	for i := 0; i < 4; i++ {
		confirm.Votes = append(confirm.Votes, payload.DPOSProposalVote{
			ProposalHash: *randomUint256(),
			Signer:       s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			Accept:       true,
			Sign:         randomSignature(),
		})
	}
	s.EqualError(ConfirmContextCheck(confirm),
		"[ConfirmContextCheck] signers less than majority count")

	// repeat votes about normal signer
	for i := 0; i < 4; i++ {
		confirm.Votes[i].Signer =
			s.arbitrators.CurrentArbitrators[1].GetNodePublicKey()
	}
	s.EqualError(ConfirmContextCheck(confirm),
		"[ConfirmContextCheck] signers less than majority count")

	for i := 0; i < 4; i++ {
		confirm.Votes[i].Signer =
			s.arbitrators.CurrentArbitrators[i].GetNodePublicKey()
	}
	s.NoError(ConfirmContextCheck(confirm))
}

func TestConfirmValidatorTestSuite(t *testing.T) {
	suite.Run(t, new(confirmValidatorTestSuite))
}

func randomUint256() *common.Uint256 {
	randBytes := make([]byte, 32)
	rand.Read(randBytes)

	result, _ := common.Uint256FromBytes(randBytes)
	return result
}

func randomPublicKey() []byte {
	_, pub, _ := crypto.GenerateKeyPair()
	result, _ := pub.EncodePoint(true)
	return result
}

func randomSignature() []byte {
	randBytes := make([]byte, 64)
	rand.Read(randBytes)

	return randBytes
}
