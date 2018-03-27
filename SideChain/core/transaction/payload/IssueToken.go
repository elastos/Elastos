package payload

import (
	spvdb "SPVWallet/db"
	"bytes"
	"errors"
	"io"
)

const IssueTokenPayloadVersion byte = 0x00

type IssueToken struct {
	Proof *spvdb.Proof
}

func (t *IssueToken) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := t.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (t *IssueToken) Serialize(w io.Writer, version byte) error {
	err := t.Proof.Serialize(w)
	if err != nil {
		return errors.New("[IssueToken], proof serialize failed.")
	}

	return nil
}

func (t *IssueToken) Deserialize(r io.Reader, version byte) error {
	if err := t.Proof.Deserialize(r); err != nil {
		return errors.New("[IssueToken], proof deserialize failed.")
	}

	return nil
}
