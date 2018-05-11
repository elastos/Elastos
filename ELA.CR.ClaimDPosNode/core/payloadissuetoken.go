package core

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const IssueTokenPayloadVersion byte = 0x00

type PayloadIssueToken struct {
	MerkleProof []byte
}

func (t *PayloadIssueToken) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := t.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (t *PayloadIssueToken) Serialize(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, t.MerkleProof)
	if err != nil {
		return errors.New("[PayloadIssueToken], MerkleProof serialize failed.")
	}

	return nil
}

func (t *PayloadIssueToken) Deserialize(r io.Reader, version byte) error {
	var err error
	if t.MerkleProof, err = common.ReadVarBytes(r); err != nil {
		return errors.New("[PayloadIssueToken], MerkleProof deserialize failed.")
	}

	return nil
}
