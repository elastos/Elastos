package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const CRManagementVersion byte = 0x00

type CRDPOSManagement struct {
	CRManagementPublicKey []byte
	CRCommitteeDID        common.Uint168
	Signature             []byte
}

func (p *CRDPOSManagement) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *CRDPOSManagement) Serialize(w io.Writer, version byte) error {
	err := p.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, p.Signature); err != nil {
		return errors.New("Serialize error")
	}
	return nil
}

func (p *CRDPOSManagement) SerializeUnsigned(w io.Writer, version byte) error {
	if err := common.WriteVarBytes(w, p.CRManagementPublicKey); err != nil {
		return errors.New("failed to serialize CRManagementPublicKey")
	}
	if err := p.CRCommitteeDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCommitteeDID")
	}
	return nil
}

func (p *CRDPOSManagement) Deserialize(r io.Reader, version byte) error {
	err := p.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	p.Signature, err = common.ReadVarBytes(r, crypto.MaxSignatureScriptLength, "signature")
	if err != nil {
		return errors.New("Deserialize error")
	}
	return nil
}

func (p *CRDPOSManagement) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	p.CRManagementPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "CRManagementPublicKey")
	if err != nil {
		return errors.New("failed to deserialize CRManagementPublicKey")
	}
	if err = p.CRCommitteeDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCommitteeDID")
	}
	return nil
}
