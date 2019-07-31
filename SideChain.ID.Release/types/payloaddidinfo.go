package types

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const DIDInfoVersion = 0x00

type DIDHeaderInfo struct {
	Specification string `json:"specification"`
	Operation     string `json"operation"`
}

func (d *DIDHeaderInfo) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarString(w, d.Specification); err != nil {
		return errors.New("[DIDHeaderInfo], Specification serialize failed.")
	}

	if err := common.WriteVarString(w, d.Operation); err != nil {
		return errors.New("[DIDHeaderInfo], Operation serialize failed.")
	}

	return nil
}

func (d *DIDHeaderInfo) Deserialize(r io.Reader, version byte) error {
	var err error
	d.Specification, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDHeaderInfo], Specification deserialize failed.")
	}

	d.Operation, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDHeaderInfo], Operation deserialize failed.")
	}

	return nil
}

type DIDProofInfo struct {
	Type               string `json:"type"`
	VerificationMethod string `json:"verificationMethod"`
	Signature          string `json:"signature"`
}

func (d *DIDProofInfo) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarString(w, d.Type); err != nil {
		return errors.New("[DIDProofInfo], Type serialize failed.")
	}

	if err := common.WriteVarString(w, d.VerificationMethod); err != nil {
		return errors.New("[DIDProofInfo], VerificationMethod serialize failed.")
	}

	if err := common.WriteVarString(w, d.Signature); err != nil {
		return errors.New("[DIDProofInfo], Signature serialize failed.")
	}
	return nil
}

func (d *DIDProofInfo) Deserialize(r io.Reader, version byte) error {
	var err error
	d.Type, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDProofInfo], Type deserialize failed.")
	}

	d.VerificationMethod, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDProofInfo], VerificationMethod deserialize failed.")
	}

	d.Signature, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDProofInfo], Signature deserialize failed.")
	}
	return nil
}

type PayloadDID struct {
	Header  DIDHeaderInfo `json:"header"`
	Payload string        `json:"payload"`
	Proof   DIDProofInfo  `json:"proof"`
}

func (p *PayloadDID) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	p.Serialize(buf, DIDInfoVersion)
	return buf.Bytes()
}

func (p *PayloadDID) Serialize(w io.Writer, version byte) error {
	if err := p.Header.Serialize(w, DIDInfoVersion); err != nil {
		return errors.New("[PayloadDID], Header serialize failed," + err.Error())
	}

	if err := common.WriteVarString(w, p.Payload); err != nil {
		return errors.New("[PayloadDID], Payload serialize failed," + err.Error())
	}

	if err := p.Proof.Serialize(w, DIDInfoVersion); err != nil {
		return errors.New("[PayloadDID], Proof serialize failed," + err.Error())
	}

	return nil
}

func (p *PayloadDID) Deserialize(r io.Reader, version byte) error {

	if err := p.Header.Deserialize(r, version); err != nil {
		return errors.New("[DIDInfo], Header deserialize failed.")
	}

	payload, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDInfo], payload deserialize failed.")
	}
	p.Payload = payload

	if err := p.Proof.Deserialize(r, version); err != nil {
		return errors.New("[DIDInfo], Proof deserialize failed.")
	}
	return nil
}

func (p *PayloadDID) GetData() []byte {
	return p.Data(DIDInfoVersion)
}
