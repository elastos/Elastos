package payload

import (
	"io"
)

const IssueTokenPayloadVersion byte = 0x00

type IssueToken struct {
}

func (t *IssueToken) Data(version byte) []byte {
	return []byte{0}
}

func (t *IssueToken) Serialize(w io.Writer, version byte) error {
	return nil
}

func (t *IssueToken) Deserialize(r io.Reader, version byte) error {
	return nil
}
