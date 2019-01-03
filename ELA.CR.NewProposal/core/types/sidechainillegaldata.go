package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

type SidechainIllegalEvidence struct {
	DataHash common.Uint256
}

type SidechainIllegalData struct {
	IllegalType         IllegalDataType
	Height              uint32
	IllegalSigner       string
	Evidence            SidechainIllegalEvidence
	CompareEvidence     SidechainIllegalEvidence
	GenesisBlockAddress string
	Signs               [][]byte

	hash *common.Uint256
}

func (s *SidechainIllegalEvidence) Serialize(w io.Writer) error {
	if err := s.DataHash.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (s *SidechainIllegalEvidence) Deserialize(r io.Reader) error {
	if err := s.DataHash.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (s *SidechainIllegalData) Type() IllegalDataType {
	return s.IllegalType
}

func (s *SidechainIllegalData) GetBlockHeight() uint32 {
	return s.Height
}

func (s *SidechainIllegalData) SerializeUnsigned(w io.Writer) error {
	if err := common.WriteUint8(w, byte(s.IllegalType)); err != nil {
		return err
	}

	if err := common.WriteUint32(w, s.Height); err != nil {
		return err
	}

	if err := common.WriteVarString(w, s.IllegalSigner); err != nil {
		return err
	}

	if err := s.Evidence.Serialize(w); err != nil {
		return err
	}

	if err := s.CompareEvidence.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteVarString(w, s.GenesisBlockAddress); err != nil {
		return err
	}

	return nil
}

func (s *SidechainIllegalData) Serialize(w io.Writer) error {
	if err := s.SerializeUnsigned(w); err != nil {
		return err
	}

	if err := common.WriteVarUint(w, uint64(len(s.Signs))); err != nil {
		return err
	}
	for _, v := range s.Signs {
		if err := common.WriteVarBytes(w, v); err != nil {
			return err
		}
	}

	return nil
}

func (s *SidechainIllegalData) DeserializeUnsigned(r io.Reader) error {
	var err error

	var illegalType uint8
	if illegalType, err = common.ReadUint8(r); err != nil {
		return err
	}
	s.IllegalType = IllegalDataType(illegalType)

	if s.Height, err = common.ReadUint32(r); err != nil {
		return err
	}

	if s.IllegalSigner, err = common.ReadVarString(r); err != nil {
		return err
	}

	if err = s.Evidence.Deserialize(r); err != nil {
		return err
	}

	if err = s.CompareEvidence.Deserialize(r); err != nil {
		return err
	}

	if s.GenesisBlockAddress, err = common.ReadVarString(r); err != nil {
		return err
	}

	return nil
}

func (s *SidechainIllegalData) Deserialize(r io.Reader) error {
	var err error
	if err = s.DeserializeUnsigned(r); err != nil {
		return err
	}

	var signLen uint64
	if signLen, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}
	s.Signs = make([][]byte, signLen)
	for i := 0; i < int(signLen); i++ {
		s.Signs[i], err = common.ReadVarBytes(r, crypto.SignatureLength, "Signature")
		if err != nil {
			return err
		}
	}

	return nil
}

func (s *SidechainIllegalData) Hash() common.Uint256 {
	if s.hash == nil {
		buf := new(bytes.Buffer)
		s.Serialize(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		s.hash = &hash
	}
	return *s.hash
}
