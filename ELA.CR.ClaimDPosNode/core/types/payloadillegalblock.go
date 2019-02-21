package types

import (
	"bytes"
	"io"
)

const PayloadIllegalBlockVersion byte = 0x00

type PayloadIllegalBlock struct {
	DposIllegalBlocks
}

func (p *PayloadIllegalBlock) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *PayloadIllegalBlock) Serialize(w io.Writer, version byte) error {
	if err := p.DposIllegalBlocks.Serialize(w); err != nil {
		return err
	}
	return nil
}

func (p *PayloadIllegalBlock) Deserialize(r io.Reader, version byte) error {
	if err := p.DposIllegalBlocks.Deserialize(r); err != nil {
		return err
	}
	return nil
}
