package types

import (
	"bytes"
	"io"
)

const PayloadSidechainIllegalDataVersion byte = 0x00

type PayloadSidechainIllegalData struct {
	SidechainIllegalData
}

func (p *PayloadSidechainIllegalData) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *PayloadSidechainIllegalData) Serialize(w io.Writer, version byte) error {
	if err := p.SidechainIllegalData.Serialize(w); err != nil {
		return err
	}
	return nil
}

func (p *PayloadSidechainIllegalData) Deserialize(r io.Reader, version byte) error {
	if err := p.SidechainIllegalData.Deserialize(r); err != nil {
		return err
	}
	return nil
}
