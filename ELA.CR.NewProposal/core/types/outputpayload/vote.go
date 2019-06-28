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
	Delegate VoteType = 0x00
	CRC      VoteType = 0x01
)

type VoteType byte

type VoteContent struct {
	VoteType   VoteType
	Candidates [][]byte
	Votes      []common.Fixed64
}

func (vc *VoteContent) Serialize(w io.Writer, version byte) error {
	if _, err := w.Write([]byte{byte(vc.VoteType)}); err != nil {
		return err
	}
	if err := common.WriteVarUint(w, uint64(len(vc.Candidates))); err != nil {
		return err
	}
	for _, candidate := range vc.Candidates {
		if err := common.WriteVarBytes(w, candidate); err != nil {
			return err
		}
	}
	if version == byte(1) {
		if err := common.WriteVarUint(w, uint64(len(vc.Votes))); err != nil {
			return err
		}
		for _, v := range vc.Votes {
			if err := v.Serialize(w); err != nil {
				return err
			}
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
		candidate, err := common.ReadVarBytes(r, crypto.COMPRESSEDLEN, "producer")
		if err != nil {
			return err
		}
		vc.Candidates = append(vc.Candidates, candidate)
	}

	if version == byte(1) {
		votesCount, err := common.ReadVarUint(r, 0)
		if err != nil {
			return err
		}

		for i := uint64(0); i < votesCount; i++ {
			var vote common.Fixed64
			if err := vote.Deserialize(r); err != nil {
				return err
			}
			vc.Votes = append(vc.Votes, vote)
		}
	}

	return nil
}

func (vc VoteContent) String() string {
	if len(vc.Votes) == 0 {
		candidates := make([]string, 0)
		for _, c := range vc.Candidates {
			candidates = append(candidates, common.BytesToHexString(c))
		}

		return fmt.Sprint("Content: {\n\t\t\t\t",
			"VoteType: ", vc.VoteType, "\n\t\t\t\t",
			"Candidates: ", candidates, "}\n\t\t\t\t")
	}

	type candidateVote struct {
		Candidate string
		Vote      string
	}

	candidateVotes := make([]candidateVote, 0, len(vc.Candidates))
	for i := 0; i < len(vc.Candidates); i++ {
		candidateVotes = append(candidateVotes, candidateVote{
			Candidate: common.BytesToHexString(vc.Candidates[i]),
			Vote:      vc.Votes[i].String(),
		})
	}

	return fmt.Sprint("Content: {\n\t\t\t\t",
		"VoteType: ", vc.VoteType, "\n\t\t\t\t",
		"CandidateVotes: ", candidateVotes, "}\n\t\t\t\t")
}

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
	if o.Version != byte(0) && o.Version != byte(1) {
		return errors.New("invalid vote version")
	}
	typeMap := make(map[VoteType]struct{})
	for _, content := range o.Contents {
		if _, exists := typeMap[content.VoteType]; exists {
			return errors.New("duplicate vote type")
		}
		typeMap[content.VoteType] = struct{}{}

		if len(content.Candidates) == 0 || len(content.Candidates) > MaxVoteProducersPerTransaction {
			return errors.New("invalid public key count")
		}
		// only use Delegate and CRC as a vote type for now
		if content.VoteType != Delegate && content.VoteType != CRC {
			return errors.New("invalid vote type")
		}

		candidateMap := make(map[string]struct{})
		for _, candidate := range content.Candidates {
			c := common.BytesToHexString(candidate)
			if _, exists := candidateMap[c]; exists {
				return errors.New("duplicate candidate")
			}
			candidateMap[c] = struct{}{}
		}

		if o.Version == byte(1) && len(content.Candidates) != len(content.Votes) {
			return errors.New("invalid candidates and votes count")
		}
	}

	return nil
}

func (o VoteOutput) String() string {
	return fmt.Sprint("Vote: {\n\t\t\t",
		"Version: ", o.Version, "\n\t\t\t",
		"Contents: ", o.Contents, "\n\t\t\t}")
}
