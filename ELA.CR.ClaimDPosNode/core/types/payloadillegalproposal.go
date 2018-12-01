package types

import (
	"bytes"
	"io"
)

const PayloadIllegalProposalVersion byte = 0x00

type PayloadIllegalProposal struct {
	DposIllegalProposals
}

func (p *PayloadIllegalProposal) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *PayloadIllegalProposal) Serialize(w io.Writer, version byte) error {
	if err := p.DposIllegalProposals.Serialize(w); err != nil {
		return err
	}
	return nil
}

func (p *PayloadIllegalProposal) Deserialize(r io.Reader, version byte) error {
	if err := p.DposIllegalProposals.Deserialize(r); err != nil {
		return err
	}
	return nil
}
