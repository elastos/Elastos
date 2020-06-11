package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	CRManagementVersion byte               = 0x00
	CRManagement        DPOSManagementType = 0x0100
)

type DPOSManagementType uint16

func (t DPOSManagementType) Name() string {
	switch t {
	case CRManagement:
		return "CRManagement"
	default:
		return "Unknown"
	}
}

type CRDPOSManagement struct {
	ManagementType        DPOSManagementType
	CRManagementPublicKey []byte
	CRCommitteeDID        common.Uint168
	Signature             []byte
}

func (p *CRDPOSManagement) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.SerializeUnsigned(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (p *CRDPOSManagement) SerializeUnsigned(w io.Writer, version byte) error {
	switch p.ManagementType {
	case CRManagement:
		return p.SerializeUnsignedCRManagement(w, version)
	default:
		return p.SerializeUnsignedCRManagement(w, version)
	}
}

func (p *CRDPOSManagement) SerializeUnsignedCRManagement(w io.Writer, version byte) error {
	if err := common.WriteElement(w, p.ManagementType); err != nil {
		return errors.New("failed to serialize ManagementType")
	}
	if err := common.WriteVarBytes(w, p.CRManagementPublicKey); err != nil {
		return errors.New("failed to serialize CRManagementPublicKey")
	}
	if err := p.CRCommitteeDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCommitteeDID")
	}
	if err := common.WriteVarBytes(w, p.Signature); err != nil {
		return errors.New("failed to serialize Signature")
	}
	return nil
}

func (p *CRDPOSManagement) Serialize(w io.Writer, version byte) error {
	switch p.ManagementType {
	case CRManagement:
		return p.SerializeCRManagement(w, version)
	default:
		return p.SerializeCRManagement(w, version)
	}
}

func (p *CRDPOSManagement) SerializeCRManagement(w io.Writer, version byte) error {
	if err := p.SerializeUnsigned(w, version); err != nil {
		return err
	}
	if err := common.WriteVarBytes(w, p.Signature); err != nil {
		return err
	}
	if err := p.CRCommitteeDID.Serialize(w); err != nil {
		return errors.New("failed to serialize CRCommitteeDID")
	}
	return nil
}

func (p *CRDPOSManagement) Deserialize(r io.Reader, version byte) error {
	if err := common.ReadElement(r, &p.ManagementType); err != nil {
		return errors.New("[CRDPOSManagement], ManagementType deserialize failed")
	}

	switch p.ManagementType {
	case CRManagement:
		return p.DeserializeCRManagement(r, version)
	default:
		return p.DeserializeCRManagement(r, version)
	}
}

func (p *CRDPOSManagement) DeserializeCRManagement(r io.Reader, version byte) error {
	if err := p.DeserializeUnSigned(r, version); err != nil {
		return err
	}

	// owner signature
	sign, err := common.ReadVarBytes(r, crypto.SignatureLength, "sign data")
	if err != nil {
		return err
	}
	p.Signature = sign

	if err := p.CRCommitteeDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCommitteeDID")
	}
	// cr signature
	crSign, err := common.ReadVarBytes(r, crypto.SignatureLength, "CR sign data")
	if err != nil {
		return err
	}
	p.Signature = crSign
	return nil
}

func (p *CRDPOSManagement) DeserializeUnSigned(r io.Reader, version byte) error {
	switch p.ManagementType {
	case CRManagement:
		return p.DeserializeUnSignedCRManagement(r, version)
	default:
		return p.DeserializeUnSignedCRManagement(r, version)
	}
}

func (p *CRDPOSManagement) DeserializeUnSignedCRManagement(r io.Reader, version byte) error {
	err := common.ReadElement(r, &p.ManagementType)
	if err != nil {
		return errors.New("[CRDPOSManagement], ManagementType deserialize failed")
	}
	p.CRManagementPublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "CRManagementPublicKey")
	if err != nil {
		return errors.New("failed to deserialize CRManagementPublicKey")
	}

	if err = p.CRCommitteeDID.Deserialize(r); err != nil {
		return errors.New("failed to deserialize CRCommitteeDID")
	}

	p.Signature, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "Signature")
	if err != nil {
		return errors.New("failed to deserialize Signature")
	}

	return nil
}
