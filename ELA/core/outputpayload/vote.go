package outputpayload

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	Delegate VoteType = 0x00
	CRC      VoteType = 0x01
)

type VoteType byte

var VoteTypes = []VoteType{
	Delegate,
	CRC,
}

type VoteContent struct {
	VoteType   VoteType
	Candidates []common.Uint168
}

func (vc *VoteContent) Serialize(w io.Writer) error {
	if _, err := w.Write([]byte{byte(vc.VoteType)}); err != nil {
		return err
	}
	if err := common.WriteUint32(w, uint32(len(vc.Candidates))); err != nil {
		return err
	}
	for _, candidate := range vc.Candidates {
		if err := candidate.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (vc *VoteContent) Deserialize(r io.Reader) error {
	voteType, err := common.ReadBytes(r, 1)
	if err != nil {
		return err
	}
	vc.VoteType = VoteType(voteType[0])

	candidatesCount, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	for i := uint32(0); i < candidatesCount; i++ {
		var candidate common.Uint168
		if err := candidate.Deserialize(r); err != nil {
			return err
		}
		vc.Candidates = append(vc.Candidates, candidate)
	}

	return nil
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
	if err := common.WriteUint32(w, uint32(len(o.Contents))); err != nil {
		return err
	}
	for _, content := range o.Contents {
		if err := content.Serialize(w); err != nil {
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

	contentsCount, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	for i := uint32(0); i < contentsCount; i++ {
		var content VoteContent
		if err := content.Deserialize(r); err != nil {
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

	typeMap := make(map[VoteType]struct{})
	for _, content := range o.Contents {
		if _, exists := typeMap[content.VoteType]; exists {
			return errors.New("duplicate vote type")
		}
		typeMap[content.VoteType] = struct{}{}

		candidateMap := make(map[common.Uint168]struct{})
		for _, candidate := range content.Candidates {
			if _, exists := candidateMap[candidate]; exists {
				return errors.New("duplicate candidate")
			}
			candidateMap[candidate] = struct{}{}
		}
	}
	return nil
}

func (o *VoteOutput) String() string {

	return ""
}
