package outputpayload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	// maxSideProducerIDSize defines the max SideProducerID length of bytes.
	maxSideProducerIDSize = 256
)

// Mapping output payload is defined to mapping the main chain producer's owner
// public key to a side chain producer.
type Mapping struct {
	// Version indicates the version of Mapping payload.
	Version byte

	// OwnerPublicKey is the owner public key of the main chain producer.
	OwnerPublicKey []byte

	// SideProducerID indicates a piece of data represent the identity of the
	// side chain producer, whether it is a public key or address etc.
	SideProducerID []byte

	// Signature represents the signature of the mapping payload content.
	Signature []byte
}

func (m *Mapping) Data() []byte {
	buf := new(bytes.Buffer)
	m.serializeContent(buf)
	return buf.Bytes()
}

func (m *Mapping) serializeContent(w io.Writer) error {
	if err := common.WriteUint8(w, m.Version); err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, m.OwnerPublicKey); err != nil {
		return err
	}

	return common.WriteVarBytes(w, m.SideProducerID)
}

func (m *Mapping) Serialize(w io.Writer) error {
	if err := m.serializeContent(w); err != nil {
		return err
	}

	return common.WriteVarBytes(w, m.Signature)
}

func (m *Mapping) Deserialize(r io.Reader) error {
	var err error
	m.Version, err = common.ReadUint8(r)
	if err != nil {
		return err
	}

	m.OwnerPublicKey, err = common.ReadVarBytes(r, crypto.COMPRESSEDLEN,
		"OwnerPublicKey")
	if err != nil {
		return err
	}

	m.SideProducerID, err = common.ReadVarBytes(r, maxSideProducerIDSize,
		"SideProducerID")
	if err != nil {
		return err
	}

	m.Signature, err = common.ReadVarBytes(r, crypto.SignatureLength,
		"Signature")
	return err
}

func (m *Mapping) GetVersion() byte {
	return m.Version
}

func (m *Mapping) Validate() error {
	pubKey, err := crypto.DecodePoint(m.OwnerPublicKey)
	if err != nil {
		return errors.New("mapping invalid OwnerPublicKey")
	}

	err = crypto.Verify(*pubKey, m.Data(), m.Signature)
	if err != nil {
		return errors.New("invalid content signature")
	}

	return nil
}
