// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"bytes"
	"math/rand"
	"path/filepath"
	"strconv"
	"testing"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/suite"
)

type txValidatorSpecialTxTestSuite struct {
	suite.Suite

	originalLedger     *Ledger
	arbitrators        *state.ArbitratorsMock
	arbitratorsPriKeys [][]byte
	Chain              *BlockChain
}

func init() {
	testing.Init()
}

func (s *txValidatorSpecialTxTestSuite) SetupSuite() {
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	arbitratorsPrivateKeys := []string{
		"e372ca1032257bb4be1ac99c4861ec542fd55c25c37f5f58ba8b177850b3fdeb",
		"e6deed7e23406e2dce7b01e85bcb33872a47b6200ca983fcf0540dff284923b0",
		"4441968d02a5df4dbc08ca11da2acc86c980e5fe9ff250450a80fd7421d2b0f1",
		"0b14a04e203301809feccc61dbf4e745203a3263d29a4b4091aaa138ba5fb26d",
		"0c11ebca60af2a09ac13dd84fd29c03b99cd086a08a69a9e5b87255fd9cf2eee",
		"ad44a6d5a5d1f7cafa2fa82c719108e9814ff5c71078e1cafa9f734343a2f806",
	}
	log.NewDefault(test.NodeLogPath, 0, 0, 0)

	s.arbitrators = &state.ArbitratorsMock{
		CurrentArbitrators: make([]state.ArbiterMember, 0),
		MajorityCount:      3,
	}
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		ar, _ := state.NewOriginArbiter(state.Origin, a)
		s.arbitrators.CurrentArbitrators = append(
			s.arbitrators.CurrentArbitrators, ar)
	}
	s.arbitrators.Snapshot = []*state.CheckPoint{
		{
			CurrentArbitrators: s.arbitrators.CurrentArbitrators,
		},
	}

	for _, v := range arbitratorsPrivateKeys {
		a, _ := common.HexStringToBytes(v)
		s.arbitratorsPriKeys = append(s.arbitratorsPriKeys, a)
	}

	chainStore, err := NewChainStore(filepath.Join(test.DataPath, "special"), &config.DefaultParams)
	if err != nil {
		s.Error(err)
	}
	s.Chain, err = New(chainStore, &config.DefaultParams,
		state.NewState(&config.DefaultParams, nil,
			nil), nil)
	if err != nil {
		s.Error(err)
	}

	s.originalLedger = DefaultLedger
	DefaultLedger = &Ledger{Arbitrators: s.arbitrators}
}

func (s *txValidatorSpecialTxTestSuite) TearDownSuite() {
	s.Chain.db.Close()
	DefaultLedger = s.originalLedger
}

func (s *txValidatorSpecialTxTestSuite) TestValidateProposalEvidence() {
	header := randomBlockHeader()
	buf := new(bytes.Buffer)
	header.Serialize(buf)
	evidence := &payload.ProposalEvidence{
		BlockHeight: header.Height + 1, //different from header.Height
		BlockHeader: buf.Bytes(),
		Proposal:    payload.DPOSProposal{},
	}

	s.EqualError(validateProposalEvidence(evidence),
		"evidence height and block height should match")

	evidence.BlockHeight = header.Height
	s.EqualError(validateProposalEvidence(evidence),
		"proposal hash and block should match")

	evidence.Proposal.BlockHash = header.Hash()
	s.NoError(validateProposalEvidence(evidence))

	// let proposal sanity and context check pass
	evidence.Proposal.Sponsor =
		s.arbitrators.CurrentArbitrators[0].GetNodePublicKey()
	evidence.Proposal.ViewOffset = rand.Uint32()
	evidence.Proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0],
		evidence.Proposal.Data())
	s.NoError(validateProposalEvidence(evidence))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckDPOSIllegalProposals() {
	header := randomBlockHeader()
	buf := new(bytes.Buffer)
	header.Serialize(buf)
	evidence := &payload.ProposalEvidence{
		BlockHeight: header.Height, //different from header.Height
		BlockHeader: buf.Bytes(),
		Proposal: payload.DPOSProposal{
			Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			BlockHash:  header.Hash(),
			ViewOffset: rand.Uint32(),
		},
	}
	evidence.Proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0],
		evidence.Proposal.Data())
	s.NoError(validateProposalEvidence(evidence))

	illegalProposals := &payload.DPOSIllegalProposals{
		Evidence:        *evidence,
		CompareEvidence: *evidence,
	}
	s.EqualError(CheckDPOSIllegalProposals(illegalProposals),
		"proposals can not be same")

	header2 := randomBlockHeader()
	header2.Height = header.Height + 1 //make sure height is different
	buf = new(bytes.Buffer)
	header2.Serialize(buf)
	cmpEvidence := &payload.ProposalEvidence{
		BlockHeader: buf.Bytes(),
		BlockHeight: header2.Height,
		Proposal: payload.DPOSProposal{
			Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			BlockHash:  header2.Hash(),
			ViewOffset: rand.Uint32(),
		},
	}
	cmpEvidence.Proposal.Sign, _ = crypto.Sign(
		s.arbitratorsPriKeys[0], cmpEvidence.Proposal.Data())
	illegalProposals.CompareEvidence = *cmpEvidence
	s.EqualError(CheckDPOSIllegalProposals(illegalProposals),
		"should be in same height")

	header2.Height = header.Height
	buf = new(bytes.Buffer)
	header2.Serialize(buf)
	cmpEvidence.BlockHeader = buf.Bytes()
	cmpEvidence.BlockHeight = header2.Height
	cmpEvidence.Proposal.ViewOffset =
		evidence.Proposal.ViewOffset + 1 //make sure view offset not the same
	cmpEvidence.Proposal.BlockHash = header2.Hash()
	cmpEvidence.Proposal.Sign, _ = crypto.Sign(
		s.arbitratorsPriKeys[0], cmpEvidence.Proposal.Data())
	illegalProposals.CompareEvidence = *cmpEvidence

	asc := evidence.Proposal.Hash().Compare(cmpEvidence.Proposal.Hash()) < 0
	if asc {
		illegalProposals.Evidence = *cmpEvidence
		illegalProposals.CompareEvidence = *evidence
	}
	s.EqualError(CheckDPOSIllegalProposals(illegalProposals),
		"evidence order error")

	if asc {
		illegalProposals.Evidence = *evidence
		illegalProposals.CompareEvidence = *cmpEvidence
	} else {
		illegalProposals.Evidence = *cmpEvidence
		illegalProposals.CompareEvidence = *evidence
	}
	s.EqualError(CheckDPOSIllegalProposals(illegalProposals),
		"should in same view")

	cmpEvidence.Proposal.ViewOffset = evidence.Proposal.ViewOffset
	cmpEvidence.Proposal.Sign, _ = crypto.Sign(
		s.arbitratorsPriKeys[0], cmpEvidence.Proposal.Data())
	if evidence.Proposal.Hash().Compare(cmpEvidence.Proposal.Hash()) < 0 {
		illegalProposals.Evidence = *evidence
		illegalProposals.CompareEvidence = *cmpEvidence
	} else {
		illegalProposals.Evidence = *cmpEvidence
		illegalProposals.CompareEvidence = *evidence
	}
	s.NoError(CheckDPOSIllegalProposals(illegalProposals))
}

func (s *txValidatorSpecialTxTestSuite) TestValidateVoteEvidence() {
	header := randomBlockHeader()
	buf := new(bytes.Buffer)
	header.Serialize(buf)
	evidence := &payload.VoteEvidence{
		ProposalEvidence: payload.ProposalEvidence{
			BlockHeight: header.Height,
			BlockHeader: buf.Bytes(),
			Proposal: payload.DPOSProposal{
				BlockHash:  header.Hash(),
				Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
				ViewOffset: rand.Uint32(),
			},
		},
		Vote: payload.DPOSProposalVote{},
	}
	evidence.Proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0],
		evidence.Proposal.Data())

	s.EqualError(validateVoteEvidence(evidence),
		"vote and proposal should match")

	evidence.Vote.ProposalHash = evidence.Proposal.Hash()
	s.NoError(validateVoteEvidence(evidence), "vote verify error")

	evidence.Vote.Signer =
		s.arbitrators.CurrentArbitrators[1].GetNodePublicKey()
	evidence.Vote.Accept = true
	evidence.Vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[1],
		evidence.Vote.Data())
	s.NoError(validateVoteEvidence(evidence))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckDPOSIllegalVotes_SameProposal() {
	header := randomBlockHeader()
	buf := new(bytes.Buffer)
	header.Serialize(buf)
	evidence := &payload.VoteEvidence{
		ProposalEvidence: payload.ProposalEvidence{
			BlockHeight: header.Height,
			BlockHeader: buf.Bytes(),
			Proposal: payload.DPOSProposal{
				BlockHash:  header.Hash(),
				Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
				ViewOffset: rand.Uint32(),
			},
		},
		Vote: payload.DPOSProposalVote{
			Signer: s.arbitrators.CurrentArbitrators[1].GetNodePublicKey(),
			Accept: true,
		},
	}
	evidence.Proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0],
		evidence.Proposal.Data())
	evidence.Vote.ProposalHash = evidence.Proposal.Hash()
	evidence.Vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[1],
		evidence.Vote.Data())

	illegalVotes := &payload.DPOSIllegalVotes{
		Evidence:        *evidence,
		CompareEvidence: *evidence,
	}
	s.EqualError(CheckDPOSIllegalVotes(illegalVotes),
		"votes can not be same")

	//create compare evidence with the same proposal
	cmpEvidence := &payload.VoteEvidence{
		ProposalEvidence: evidence.ProposalEvidence,
		Vote: payload.DPOSProposalVote{
			Signer:       s.arbitrators.CurrentArbitrators[1].GetNodePublicKey(),
			Accept:       false,
			ProposalHash: evidence.Proposal.Hash(),
		},
	}
	cmpEvidence.Vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[1],
		cmpEvidence.Vote.Data())

	asc := evidence.Vote.Hash().Compare(cmpEvidence.Vote.Hash()) < 0
	if asc {
		illegalVotes.Evidence = *cmpEvidence
		illegalVotes.CompareEvidence = *evidence
	} else {
		illegalVotes.Evidence = *evidence
		illegalVotes.CompareEvidence = *cmpEvidence
	}
	s.EqualError(CheckDPOSIllegalVotes(illegalVotes),
		"evidence order error")

	if asc {
		illegalVotes.Evidence = *evidence
		illegalVotes.CompareEvidence = *cmpEvidence
	} else {
		illegalVotes.Evidence = *cmpEvidence
		illegalVotes.CompareEvidence = *evidence
	}
	s.NoError(CheckDPOSIllegalVotes(illegalVotes))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckDPOSIllegalVotes_DiffProposal() {
	header := randomBlockHeader()
	buf := new(bytes.Buffer)
	header.Serialize(buf)
	evidence := &payload.VoteEvidence{
		ProposalEvidence: payload.ProposalEvidence{
			BlockHeight: header.Height,
			BlockHeader: buf.Bytes(),
			Proposal: payload.DPOSProposal{
				BlockHash:  header.Hash(),
				Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
				ViewOffset: rand.Uint32(),
			},
		},
		Vote: payload.DPOSProposalVote{
			Signer: s.arbitrators.CurrentArbitrators[1].GetNodePublicKey(),
			Accept: true,
		},
	}
	s.updateEvidenceSigns(evidence, s.arbitratorsPriKeys[0], s.arbitratorsPriKeys[1])

	//create compare evidence with the different proposal
	header2 := randomBlockHeader()
	header2.Height = header.Height + 1 //make sure height is different
	buf = new(bytes.Buffer)
	header2.Serialize(buf)
	cmpEvidence := &payload.VoteEvidence{
		ProposalEvidence: payload.ProposalEvidence{
			BlockHeight: header2.Height,
			BlockHeader: buf.Bytes(),
			Proposal: payload.DPOSProposal{
				BlockHash:  header2.Hash(),
				Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
				ViewOffset: rand.Uint32(),
			},
		},
		Vote: payload.DPOSProposalVote{
			Signer:       s.arbitrators.CurrentArbitrators[1].GetNodePublicKey(),
			Accept:       false,
			ProposalHash: evidence.Proposal.Hash(),
		},
	}
	s.updateEvidenceSigns(cmpEvidence, s.arbitratorsPriKeys[0],
		s.arbitratorsPriKeys[1])

	illegalVotes := &payload.DPOSIllegalVotes{
		Evidence:        *evidence,
		CompareEvidence: *cmpEvidence,
	}
	s.EqualError(CheckDPOSIllegalVotes(illegalVotes),
		"should be in same height")

	header2.Height = header.Height
	buf = new(bytes.Buffer)
	header2.Serialize(buf)
	cmpEvidence.BlockHeight = header2.Height
	cmpEvidence.BlockHeader = buf.Bytes()
	cmpEvidence.Proposal.BlockHash = header2.Hash()
	cmpEvidence.Proposal.Sponsor = //set different sponsor
		s.arbitrators.CurrentArbitrators[2].GetNodePublicKey()
	s.updateEvidenceSigns(cmpEvidence, s.arbitratorsPriKeys[2],
		s.arbitratorsPriKeys[1])
	if evidence.Vote.Hash().Compare(cmpEvidence.Vote.Hash()) < 0 {
		illegalVotes.Evidence = *evidence
		illegalVotes.CompareEvidence = *cmpEvidence
	} else {
		illegalVotes.Evidence = *cmpEvidence
		illegalVotes.CompareEvidence = *evidence
	}
	s.EqualError(CheckDPOSIllegalVotes(illegalVotes),
		"should be same sponsor")

	// set different view offset
	cmpEvidence.Proposal.Sponsor =
		s.arbitrators.CurrentArbitrators[0].GetNodePublicKey()
	cmpEvidence.Proposal.ViewOffset = evidence.Proposal.ViewOffset + 1
	s.updateEvidenceSigns(cmpEvidence, s.arbitratorsPriKeys[0],
		s.arbitratorsPriKeys[1])
	if evidence.Vote.Hash().Compare(cmpEvidence.Vote.Hash()) < 0 {
		illegalVotes.Evidence = *evidence
		illegalVotes.CompareEvidence = *cmpEvidence
	} else {
		illegalVotes.Evidence = *cmpEvidence
		illegalVotes.CompareEvidence = *evidence
	}
	s.EqualError(CheckDPOSIllegalVotes(illegalVotes),
		"should in same view")

	// let check method pass
	cmpEvidence.Proposal.ViewOffset = evidence.Proposal.ViewOffset
	s.updateEvidenceSigns(cmpEvidence, s.arbitratorsPriKeys[0],
		s.arbitratorsPriKeys[1])
	if evidence.Vote.Hash().Compare(cmpEvidence.Vote.Hash()) < 0 {
		illegalVotes.Evidence = *evidence
		illegalVotes.CompareEvidence = *cmpEvidence
	} else {
		illegalVotes.Evidence = *cmpEvidence
		illegalVotes.CompareEvidence = *evidence
	}
	s.NoError(CheckDPOSIllegalVotes(illegalVotes))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckDPOSIllegalBlocks() {
	header := randomBlockHeader()
	buf := new(bytes.Buffer)
	header.Serialize(buf)
	evidence := &payload.BlockEvidence{
		Header:       buf.Bytes(),
		BlockConfirm: []byte{},
		Signers:      [][]byte{},
	}

	illegalBlocks := &payload.DPOSIllegalBlocks{
		CoinType:        payload.ELACoin,
		BlockHeight:     rand.Uint32(),
		Evidence:        *evidence,
		CompareEvidence: *evidence,
	}
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"blocks can not be same")

	header2 := randomBlockHeader()
	buf = new(bytes.Buffer)
	header2.Serialize(buf)
	cmpEvidence := &payload.BlockEvidence{
		Header:       buf.Bytes(),
		BlockConfirm: []byte{},
		Signers:      [][]byte{},
	}

	asc := bytes.Compare(evidence.Header, cmpEvidence.Header) < 0
	if asc {
		illegalBlocks.Evidence = *cmpEvidence
		illegalBlocks.CompareEvidence = *evidence
	} else {
		illegalBlocks.Evidence = *evidence
		illegalBlocks.CompareEvidence = *cmpEvidence
	}
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"evidence order error")

	illegalBlocks.CoinType = payload.CoinType(1) //
	if asc {
		illegalBlocks.Evidence = *evidence
		illegalBlocks.CompareEvidence = *cmpEvidence
	} else {
		illegalBlocks.Evidence = *cmpEvidence
		illegalBlocks.CompareEvidence = *evidence
	}
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"unknown coin type")

	illegalBlocks.CoinType = payload.ELACoin
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"block header height should be same")

	// compare evidence height is different from illegal block height
	illegalBlocks.BlockHeight = header.Height
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"block header height should be same")

	header2.Height = header.Height
	buf = new(bytes.Buffer)
	header2.Serialize(buf)
	cmpEvidence.Header = buf.Bytes()
	asc = common.BytesToHexString(evidence.Header) <
		common.BytesToHexString(cmpEvidence.Header)
	if asc {
		illegalBlocks.Evidence = *evidence
		illegalBlocks.CompareEvidence = *cmpEvidence
	} else {
		illegalBlocks.Evidence = *cmpEvidence
		illegalBlocks.CompareEvidence = *evidence
	}
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"EOF")

	// fill confirms of evidences
	confirm := &payload.Confirm{
		Proposal: payload.DPOSProposal{
			Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			BlockHash:  header.Hash(),
			ViewOffset: rand.Uint32(),
		},
		Votes: []payload.DPOSProposalVote{},
	}
	cmpConfirm := &payload.Confirm{
		Proposal: payload.DPOSProposal{
			Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			BlockHash:  header2.Hash(),
			ViewOffset: rand.Uint32(),
		},
		Votes: []payload.DPOSProposalVote{},
	}
	confirm.Proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0],
		confirm.Proposal.Data())
	cmpConfirm.Proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0],
		cmpConfirm.Proposal.Data())
	s.updateIllegaBlocks(confirm, evidence, cmpConfirm, cmpEvidence, asc,
		illegalBlocks)
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"[ConfirmContextCheck] signers less than majority count")

	// fill votes of confirms
	for i := 0; i < 4; i++ {
		vote := payload.DPOSProposalVote{
			ProposalHash: confirm.Proposal.Hash(),
			Signer:       s.arbitrators.CurrentArbitrators[i].GetNodePublicKey(),
			Accept:       true,
		}
		vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[i], vote.Data())
		confirm.Votes = append(confirm.Votes, vote)
	}
	for i := 1; i < 5; i++ {
		vote := payload.DPOSProposalVote{
			ProposalHash: cmpConfirm.Proposal.Hash(),
			Signer:       s.arbitrators.CurrentArbitrators[i].GetNodePublicKey(),
			Accept:       true,
		}
		vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[i], vote.Data())
		cmpConfirm.Votes = append(cmpConfirm.Votes, vote)
	}
	s.updateIllegaBlocks(confirm, evidence, cmpConfirm, cmpEvidence, asc,
		illegalBlocks)
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"confirm view offset should not be same")

	// correct view offset
	proposal := payload.DPOSProposal{
		Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
		BlockHash:  *randomUint256(),
		ViewOffset: confirm.Proposal.ViewOffset,
	}
	proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0], proposal.Data())
	cmpConfirm.Proposal = proposal
	cmpConfirm.Votes = make([]payload.DPOSProposalVote, 0)
	for i := 1; i < 5; i++ {
		vote := payload.DPOSProposalVote{
			ProposalHash: cmpConfirm.Proposal.Hash(),
			Signer:       s.arbitrators.CurrentArbitrators[i].GetNodePublicKey(),
			Accept:       true,
		}
		vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[i], vote.Data())
		cmpConfirm.Votes = append(cmpConfirm.Votes, vote)
	}
	s.updateIllegaBlocks(confirm, evidence, cmpConfirm, cmpEvidence, asc,
		illegalBlocks)
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"block and related confirm do not match")

	// correct block hash corresponding to header hash
	proposal = payload.DPOSProposal{
		Sponsor:    s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
		BlockHash:  header2.Hash(),
		ViewOffset: confirm.Proposal.ViewOffset,
	}
	proposal.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[0], proposal.Data())
	cmpConfirm.Proposal = proposal
	cmpConfirm.Votes = make([]payload.DPOSProposalVote, 0)
	for i := 1; i < 5; i++ {
		vote := payload.DPOSProposalVote{
			ProposalHash: cmpConfirm.Proposal.Hash(),
			Signer:       s.arbitrators.CurrentArbitrators[i].GetNodePublicKey(),
			Accept:       true,
		}
		vote.Sign, _ = crypto.Sign(s.arbitratorsPriKeys[i], vote.Data())
		cmpConfirm.Votes = append(cmpConfirm.Votes, vote)
	}
	s.updateIllegaBlocks(confirm, evidence, cmpConfirm, cmpEvidence, asc,
		illegalBlocks)
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"signers count it not match the count of confirm votes")

	// fill the same signers to evidences
	for _, v := range confirm.Votes {
		evidence.Signers = append(evidence.Signers, v.Signer)
		cmpEvidence.Signers = append(cmpEvidence.Signers, v.Signer)
	}
	s.updateIllegaBlocks(confirm, evidence, cmpConfirm, cmpEvidence, asc,
		illegalBlocks)
	s.EqualError(CheckDPOSIllegalBlocks(illegalBlocks),
		"signers and confirm votes do not match")

	// correct signers of compare evidence
	signers := make([][]byte, 0)
	for _, v := range cmpConfirm.Votes {
		signers = append(signers, v.Signer)
	}
	cmpEvidence.Signers = signers
	s.updateIllegaBlocks(confirm, evidence, cmpConfirm, cmpEvidence, asc,
		illegalBlocks)
	s.NoError(CheckDPOSIllegalBlocks(illegalBlocks))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckSidechainIllegalEvidence() {
	illegalData := &payload.SidechainIllegalData{
		IllegalType: payload.IllegalBlock, // set illegal type
	}
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"invalid type")

	illegalData.IllegalType = payload.SidechainIllegalProposal
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"the encodeData cann't be nil")

	illegalData.IllegalSigner = randomBytes(33)
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"the encodeData format is error")

	_, pk, _ := crypto.GenerateKeyPair()
	illegalData.IllegalSigner, _ = pk.EncodePoint(true)
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"illegal signer is not one of current arbitrators")

	illegalData.IllegalSigner = s.arbitrators.CurrentArbitrators[0].GetNodePublicKey()
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"[Uint168FromAddress] error, len != 34")

	illegalData.GenesisBlockAddress = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"insufficient signs count")

	for i := 0; i < 4; i++ {
		s, _ := crypto.Sign(s.arbitrators.CurrentArbitrators[0].GetNodePublicKey(),
			illegalData.Data(payload.SidechainIllegalDataVersion))
		illegalData.Signs = append(illegalData.Signs, s)
	}
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"evidence order error")

	// same data hash will emit order error
	evidence := &payload.SidechainIllegalEvidence{}
	cmpEvidence := &payload.SidechainIllegalEvidence{}
	evidence.DataHash = *randomUint256()
	cmpEvidence.DataHash = evidence.DataHash
	illegalData.Evidence = *evidence
	illegalData.CompareEvidence = *cmpEvidence
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"evidence order error")

	cmpEvidence.DataHash = *randomUint256()
	asc := evidence.DataHash.Compare(cmpEvidence.DataHash) < 0
	if asc {
		illegalData.Evidence = *cmpEvidence
		illegalData.CompareEvidence = *evidence
	} else {
		illegalData.Evidence = *evidence
		illegalData.CompareEvidence = *cmpEvidence
	}
	s.EqualError(CheckSidechainIllegalEvidence(illegalData),
		"evidence order error")

	if asc {
		illegalData.Evidence = *evidence
		illegalData.CompareEvidence = *cmpEvidence
	} else {
		illegalData.Evidence = *cmpEvidence
		illegalData.CompareEvidence = *evidence
	}
	s.NoError(CheckSidechainIllegalEvidence(illegalData))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckInactiveArbitrators() {
	p := &payload.InactiveArbitrators{
		Sponsor: randomPublicKey(),
	}
	tx := &types.Transaction{
		Payload: p,
		Programs: []*program.Program{
			{
				Code:      randomPublicKey(),
				Parameter: randomSignature(),
			},
		},
	}
	s.arbitrators.ActiveProducer = s.arbitrators.CurrentArbitrators

	s.EqualError(CheckInactiveArbitrators(tx),
		"sponsor is not belong to arbitrators")

	// correct sponsor
	ar, _ := state.NewOriginArbiter(state.Origin, p.Sponsor)
	s.arbitrators.CRCArbitrators = []state.ArbiterMember{ar}
	for i := 0; i < 3; i++ { // add more than InactiveEliminateCount arbiters
		p.Arbitrators = append(p.Arbitrators,
			s.arbitrators.CurrentArbitrators[i].GetNodePublicKey())
	}

	// correct number of Arbitrators
	p.Arbitrators = make([][]byte, 0)
	p.Arbitrators = append(p.Arbitrators, randomPublicKey())
	s.EqualError(CheckInactiveArbitrators(tx),
		"inactive arbitrator is not belong to arbitrators")

	// correct "Arbitrators" to be current arbitrators
	p.Arbitrators = make([][]byte, 0)
	for i := 4; i < 5; i++ {
		p.Arbitrators = append(p.Arbitrators,
			s.arbitrators.CurrentArbitrators[i].GetNodePublicKey())
	}
	s.EqualError(CheckInactiveArbitrators(tx),
		"invalid multi sign script code")

	// let "Arbitrators" has CRC arbitrators
	ar, _ = state.NewOriginArbiter(state.Origin, p.Sponsor)
	s.arbitrators.CRCArbitrators = []state.ArbiterMember{
		ar,
		s.arbitrators.CurrentArbitrators[4],
	}
	s.EqualError(CheckInactiveArbitrators(tx),
		"inactive arbiters should not include CRC")

	// set invalid redeem script
	s.arbitrators.CRCArbitrators = []state.ArbiterMember{}
	for i := 0; i < 5; i++ {
		_, pk, _ := crypto.GenerateKeyPair()
		pkBuf, _ := pk.EncodePoint(true)
		ar, _ = state.NewOriginArbiter(state.Origin, pkBuf)
		s.arbitrators.CRCArbitrators = append(s.arbitrators.CRCArbitrators, ar)
	}
	s.arbitrators.CRCArbitratorsMap = map[string]*state.Producer{}
	for _, v := range s.arbitrators.CRCArbitrators {
		s.arbitrators.CRCArbitratorsMap[common.BytesToHexString(
			v.GetNodePublicKey())] = nil
	}
	p.Sponsor = s.arbitrators.CRCArbitrators[0].GetNodePublicKey()
	var arbitrators []state.ArbiterMember
	for i := 0; i < 4; i++ {
		arbitrators = append(arbitrators, s.arbitrators.CurrentArbitrators[i])
	}
	_, pk, _ := crypto.GenerateKeyPair()
	pkBuf, _ := pk.EncodePoint(true)
	ar, _ = state.NewOriginArbiter(state.Origin, pkBuf)
	arbitrators = append(arbitrators, ar)
	tx.Programs[0].Code = s.createArbitratorsRedeemScript(arbitrators)
	s.EqualError(CheckInactiveArbitrators(tx),
		"invalid multi sign public key")

	// correct redeem script
	tx.Programs[0].Code = s.createArbitratorsRedeemScript(
		s.arbitrators.CRCArbitrators)
	s.NoError(CheckInactiveArbitrators(tx))
}

func (s *txValidatorSpecialTxTestSuite) TestCheckUpdateVersion() {
	tx := &types.Transaction{
		Programs: []*program.Program{
			{
				Code:      randomPublicKey(),
				Parameter: randomSignature(),
			},
		},
	}

	// set payload of invalid type
	tx.Payload = &payload.InactiveArbitrators{}
	s.EqualError(s.Chain.checkUpdateVersionTransaction(tx),
		"invalid payload")

	// set inactive mode off
	p := &payload.UpdateVersion{}
	tx.Payload = p
	s.EqualError(s.Chain.checkUpdateVersionTransaction(tx),
		"invalid update version height")

	// let EndHeight less than StartHeight
	p.StartHeight = 10
	p.EndHeight = p.StartHeight - 5
	s.EqualError(s.Chain.checkUpdateVersionTransaction(tx),
		"invalid update version height")

	// set invalid redeem script
	p.EndHeight = p.StartHeight + 5
	s.arbitrators.CRCArbitrators = []state.ArbiterMember{}
	for i := 0; i < 5; i++ {
		_, pk, _ := crypto.GenerateKeyPair()
		pkBuf, _ := pk.EncodePoint(true)
		ar, _ := state.NewOriginArbiter(state.Origin, pkBuf)
		s.arbitrators.CRCArbitrators = append(s.arbitrators.CRCArbitrators, ar)
	}
	s.arbitrators.CRCArbitratorsMap = map[string]*state.Producer{}
	for _, v := range s.arbitrators.CRCArbitrators {
		s.arbitrators.CRCArbitratorsMap[common.BytesToHexString(
			v.GetNodePublicKey())] = nil
	}
	var arbitrators []state.ArbiterMember
	for i := 0; i < 4; i++ {
		arbitrators = append(arbitrators, s.arbitrators.CurrentArbitrators[i])
	}
	_, pk, _ := crypto.GenerateKeyPair()
	pkBuf, _ := pk.EncodePoint(true)
	ar, _ := state.NewOriginArbiter(state.Origin, pkBuf)
	arbitrators = append(arbitrators, ar)
	tx.Programs[0].Code = s.createArbitratorsRedeemScript(arbitrators)
	s.EqualError(s.Chain.checkUpdateVersionTransaction(tx),
		"invalid multi sign public key")

	// correct redeem script
	tx.Programs[0].Code = s.createArbitratorsRedeemScript(
		s.arbitrators.CRCArbitrators)
	s.NoError(s.Chain.checkUpdateVersionTransaction(tx))
}

func TestTxValidatorSpecialTxSuite(t *testing.T) {
	suite.Run(t, new(txValidatorSpecialTxTestSuite))
}

func (s *txValidatorSpecialTxTestSuite) updateEvidenceSigns(
	evidence *payload.VoteEvidence, proposalSigner, voteSigner []byte) {
	evidence.Proposal.Sign, _ = crypto.Sign(proposalSigner,
		evidence.Proposal.Data())
	evidence.Vote.ProposalHash = evidence.Proposal.Hash()
	evidence.Vote.Sign, _ = crypto.Sign(voteSigner, evidence.Vote.Data())
}

func (s *txValidatorSpecialTxTestSuite) createArbitratorsRedeemScript(
	arbitrators []state.ArbiterMember) []byte {

	var pks []*crypto.PublicKey
	for _, v := range arbitrators {
		pk, err := crypto.DecodePoint(v.GetNodePublicKey())
		if err != nil {
			return nil
		}
		pks = append(pks, pk)
	}

	arbitratorsCount := len(arbitrators)
	minSignCount := arbitratorsCount * 2 / 3
	result, _ := contract.CreateMultiSigRedeemScript(minSignCount+1, pks)
	return result
}

func randomBlockHeader() *types.Header {
	return &types.Header{
		Version:    rand.Uint32(),
		Previous:   *randomUint256(),
		MerkleRoot: *randomUint256(),
		Timestamp:  rand.Uint32(),
		Bits:       rand.Uint32(),
		Nonce:      rand.Uint32(),
		Height:     rand.Uint32(),
		AuxPow: auxpow.AuxPow{
			AuxMerkleBranch: []common.Uint256{
				*randomUint256(),
				*randomUint256(),
			},
			AuxMerkleIndex: rand.Int(),
			ParCoinbaseTx: auxpow.BtcTx{
				Version: rand.Int31(),
				TxIn: []*auxpow.BtcTxIn{
					{
						PreviousOutPoint: auxpow.BtcOutPoint{
							Hash:  *randomUint256(),
							Index: rand.Uint32(),
						},
						SignatureScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
						Sequence:        rand.Uint32(),
					},
					{
						PreviousOutPoint: auxpow.BtcOutPoint{
							Hash:  *randomUint256(),
							Index: rand.Uint32(),
						},
						SignatureScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
						Sequence:        rand.Uint32(),
					},
				},
				TxOut: []*auxpow.BtcTxOut{
					{
						Value:    rand.Int63(),
						PkScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
					},
					{
						Value:    rand.Int63(),
						PkScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
					},
				},
				LockTime: rand.Uint32(),
			},
			ParCoinBaseMerkle: []common.Uint256{
				*randomUint256(),
				*randomUint256(),
			},
			ParMerkleIndex: rand.Int(),
			ParBlockHeader: auxpow.BtcHeader{
				Version:    rand.Uint32(),
				Previous:   *randomUint256(),
				MerkleRoot: *randomUint256(),
				Timestamp:  rand.Uint32(),
				Bits:       rand.Uint32(),
				Nonce:      rand.Uint32(),
			},
			ParentHash: *randomUint256(),
		},
	}
}

func (s *txValidatorSpecialTxTestSuite) updateIllegaBlocks(
	confirm *payload.Confirm, evidence *payload.BlockEvidence,
	cmpConfirm *payload.Confirm, cmpEvidence *payload.BlockEvidence,
	asc bool, illegalBlocks *payload.DPOSIllegalBlocks) {
	buf := new(bytes.Buffer)
	confirm.Serialize(buf)
	evidence.BlockConfirm = buf.Bytes()
	buf = new(bytes.Buffer)
	cmpConfirm.Serialize(buf)
	cmpEvidence.BlockConfirm = buf.Bytes()
	if asc {
		illegalBlocks.Evidence = *evidence
		illegalBlocks.CompareEvidence = *cmpEvidence
	} else {
		illegalBlocks.Evidence = *cmpEvidence
		illegalBlocks.CompareEvidence = *evidence
	}
}
