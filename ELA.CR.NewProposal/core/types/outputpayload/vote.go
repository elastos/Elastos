// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package outputpayload

import (
	"errors"
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	MaxVoteProducersPerTransaction = 36
)

const (
	// Delegate indicates the vote content is for producer.
	Delegate VoteType = 0x00

	// CRC indicates the vote content is for CRC.
	CRC VoteType = 0x01

	// Proposal indicates the vote content is for reject proposal.
	CRCProposal VoteType = 0x02

	// Reject indicates the vote content is for impeachment.
	CRCImpeachment VoteType = 0x03
)

// VoteType indicates the type of vote content.
type VoteType byte

const (
	// VoteProducerVersion indicates the output version only support delegate
	// vote type, and not support different votes for different candidates.
	VoteProducerVersion = 0x00

	// VoteProducerAndCRVersion indicates the output version support delegate
	// and CRC vote type, and support different votes for different candidates.
	VoteProducerAndCRVersion = 0x01
)

// CandidateVotes defines the voting information for individual candidates.
type CandidateVotes struct {
	Candidate []byte
	Votes     common.Fixed64
}

func (cv *CandidateVotes) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarBytes(w, cv.Candidate); err != nil {
		return err
	}
	if version >= VoteProducerAndCRVersion {
		if err := cv.Votes.Serialize(w); err != nil {
			return err
		}
	}
	return nil
}

func (cv *CandidateVotes) Deserialize(r io.Reader, version byte) error {
	candidate, err := common.ReadVarBytes(
		r, crypto.MaxMultiSignCodeLength, "candidate votes")
	if err != nil {
		return err
	}
	cv.Candidate = candidate

	if version >= VoteProducerAndCRVersion {
		if err := cv.Votes.Deserialize(r); err != nil {
			return err
		}
	}
	return nil
}

func (cv *CandidateVotes) String() string {
	return fmt.Sprint("Content: {\n\t\t\t\t",
		"Candidate: ", common.BytesToHexString(cv.Candidate), "\n\t\t\t\t",
		"Candidates: ", cv.Votes, "}\n\t\t\t\t")
}

// VoteContent defines the vote type and vote information of candidates.
type VoteContent struct {
	VoteType       VoteType
	CandidateVotes []CandidateVotes
}

func (vc *VoteContent) Serialize(w io.Writer, version byte) error {
	if _, err := w.Write([]byte{byte(vc.VoteType)}); err != nil {
		return err
	}
	if err := common.WriteVarUint(w, uint64(len(vc.CandidateVotes))); err != nil {
		return err
	}
	for _, candidate := range vc.CandidateVotes {
		if err := candidate.Serialize(w, version); err != nil {
			return err
		}
	}

	return nil
}

func (vc *VoteContent) Deserialize(r io.Reader, version byte) error {
	voteType, err := common.ReadBytes(r, 1)
	if err != nil {
		return err
	}
	vc.VoteType = VoteType(voteType[0])

	candidatesCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	for i := uint64(0); i < candidatesCount; i++ {
		var cv CandidateVotes
		if cv.Deserialize(r, version); err != nil {
			return err
		}
		vc.CandidateVotes = append(vc.CandidateVotes, cv)
	}

	return nil
}

func (vc VoteContent) String() string {
	candidates := make([]string, 0)
	for _, c := range vc.CandidateVotes {
		candidates = append(candidates, common.BytesToHexString(c.Candidate))
	}

	if len(vc.CandidateVotes) != 0 && vc.CandidateVotes[0].Votes == 0 {
		return fmt.Sprint("Content: {\n\t\t\t\t",
			"VoteType: ", vc.VoteType, "\n\t\t\t\t",
			"Candidates: ", candidates, "}\n\t\t\t\t")
	}

	return fmt.Sprint("Content: {\n\t\t\t\t",
		"VoteType: ", vc.VoteType, "\n\t\t\t\t",
		"CandidateVotes: ", vc.CandidateVotes, "}\n\t\t\t\t")
}

// VoteOutput defines the output payload for vote.
type VoteOutput struct {
	Version  byte
	Contents []VoteContent
}

func (o *VoteOutput) Data() []byte {
	return nil
}

func (o *VoteOutput) Serialize(w io.Writer) error {
	if _, err := w.Write([]byte{byte(o.Version)}); err != nil {
		return err
	}
	if err := common.WriteVarUint(w, uint64(len(o.Contents))); err != nil {
		return err
	}
	for _, content := range o.Contents {
		if err := content.Serialize(w, o.Version); err != nil {
			return err
		}
	}
	return nil
}

func (o *VoteOutput) Deserialize(r io.Reader) error {
	version, err := common.ReadBytes(r, 1)
	if err != nil {
		return err
	}
	o.Version = version[0]

	contentsCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	for i := uint64(0); i < contentsCount; i++ {
		var content VoteContent
		if err := content.Deserialize(r, o.Version); err != nil {
			return err
		}
		o.Contents = append(o.Contents, content)
	}

	return nil
}

func (o *VoteOutput) GetVersion() byte {
	return o.Version
}

func (o *VoteOutput) Validate() error {
	if o == nil {
		return errors.New("vote output payload is nil")
	}
	if o.Version > VoteProducerAndCRVersion {
		return errors.New("invalid vote version")
	}
	typeMap := make(map[VoteType]struct{})
	for _, content := range o.Contents {
		if _, exists := typeMap[content.VoteType]; exists {
			return errors.New("duplicate vote type")
		}
		typeMap[content.VoteType] = struct{}{}
		if len(content.CandidateVotes) == 0 || (content.VoteType != CRC &&
			len(content.CandidateVotes) > MaxVoteProducersPerTransaction) {
			return errors.New("invalid public key count")
		}
		if content.VoteType != Delegate && content.VoteType != CRC &&
			content.VoteType != CRCProposal && content.VoteType != CRCImpeachment {
			return errors.New("invalid vote type")
		}

		candidateMap := make(map[string]struct{})
		for _, cv := range content.CandidateVotes {
			c := common.BytesToHexString(cv.Candidate)
			if _, exists := candidateMap[c]; exists {
				return errors.New("duplicate candidate")
			}
			candidateMap[c] = struct{}{}

			if o.Version >= VoteProducerAndCRVersion && cv.Votes <= 0 {
				return errors.New("invalid candidate votes")
			}
		}
	}

	return nil
}

func (o VoteOutput) String() string {
	return fmt.Sprint("Vote: {\n\t\t\t",
		"Version: ", o.Version, "\n\t\t\t",
		"Contents: ", o.Contents, "\n\t\t\t}")
}
