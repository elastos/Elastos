package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const CRManagementVersion byte = 0x00

type CRCouncilMemberClaimNode struct {
	NodePublicKey               []byte
	CRCouncilCommitteeDID       common.Uint168
	CRCouncilCommitteeSignature []byte
}

func (p *CRCouncilMemberClaimNode) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *CRCouncilMemberClaimNode) Serialize(w io.Writer, version byte) error {
	err := p.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.CRCouncilCommitteeSignature); err != nil {
		return errors.New("Serialize error")
	}
	return nil
}

func (p *CRCouncilMemberClaimNode) SerializeUnsigned(w io.Writer, version byte) error {
	if err := common.WriteVarBytes(w, p.NodePublicKey); err != nil {
		return errors.New("failed to serialize NodePublicKey")
	}
	if err := p.CRCouncilCommitteeDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCouncilCommitteeDID")
	}
	return nil
}

func (p *CRCouncilMemberClaimNode) Deserialize(r io.Reader, version byte) error {
	err := p.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	p.CRCouncilCommitteeSignature, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "signature")
	if err != nil {
		return errors.New("Deserialize error")
	}
	return nil
}

func (p *CRCouncilMemberClaimNode) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	p.NodePublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "NodePublicKey")
	if err != nil {
		return errors.New("failed to deserialize NodePublicKey")
	}
	if err = p.CRCouncilCommitteeDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCouncilCommitteeDID")
	}
	return nil
}
