package core

import (
	"bytes"
	"io"
)

const PayloadIllegalVoteVersion byte = 0x00

type PayloadIllegalVote struct {
	DposIllegalVotes
}

func (p *PayloadIllegalVote) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *PayloadIllegalVote) Serialize(w io.Writer, version byte) error {
	if err := p.DposIllegalVotes.Serialize(w); err != nil {
		return err
	}
	return nil
}

func (p *PayloadIllegalVote) Deserialize(r io.Reader, version byte) error {
	if err := p.DposIllegalVotes.Deserialize(r); err != nil {
		return err
	}
	return nil
}
