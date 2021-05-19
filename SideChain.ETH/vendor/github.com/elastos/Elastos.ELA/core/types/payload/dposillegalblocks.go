package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/elanet/pact"
)

type CoinType uint32

const (
	ELACoin = CoinType(0)

	IllegalBlockVersion byte = 0x00
)

type BlockEvidence struct {
	Header       []byte
	BlockConfirm []byte
	Signers      [][]byte

	hash *common.Uint256
}

type DPOSIllegalBlocks struct {
	CoinType        CoinType
	BlockHeight     uint32
	Evidence        BlockEvidence
	CompareEvidence BlockEvidence

	hash *common.Uint256
}

func (b *BlockEvidence) SerializeUnsigned(w io.Writer) error {
	if err := common.WriteVarBytes(w, b.Header); err != nil {
		return err
	}
	return nil
}

func (b *BlockEvidence) SerializeOthers(w io.Writer) error {
	if err := common.WriteVarBytes(w, b.BlockConfirm); err != nil {
		return err
	}

	if err := common.WriteVarUint(w, uint64(len(b.Signers))); err != nil {
		return err
	}

	for _, v := range b.Signers {
		if err := common.WriteVarBytes(w, v); err != nil {
			return err
		}
	}

	return nil
}

func (b *BlockEvidence) Serialize(w io.Writer) error {
	if err := b.SerializeUnsigned(w); err != nil {
		return err
	}

	if err := b.SerializeOthers(w); err != nil {
		return err
	}

	return nil
}

func (b *BlockEvidence) DeserializeUnsigned(r io.Reader) error {
	var err error
	if b.Header, err = common.ReadVarBytes(r, pact.MaxBlockSize,
		"block data"); err != nil {
		return err
	}
	return nil
}

func (b *BlockEvidence) DeserializeOthers(r io.Reader) (err error) {
	if b.BlockConfirm, err = common.ReadVarBytes(r, pact.MaxBlockSize,
		"confirm data"); err != nil {
		return err
	}

	var len uint64
	if len, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}

	b.Signers = make([][]byte, 0, len)
	for i := uint64(0); i < len; i++ {
		var signer []byte
		if signer, err = common.ReadVarBytes(r, crypto.COMPRESSEDLEN,
			"public key"); err != nil {
			return err
		}
		b.Signers = append(b.Signers, signer)
	}

	return nil
}

func (b *BlockEvidence) Deserialize(r io.Reader) error {
	if err := b.DeserializeUnsigned(r); err != nil {
		return err
	}

	if err := b.DeserializeOthers(r); err != nil {
		return err
	}

	return nil
}

func (b *BlockEvidence) BlockHash() common.Uint256 {
	if b.hash == nil {
		buf := new(bytes.Buffer)
		b.Serialize(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		b.hash = &hash
	}
	return *b.hash
}

func (d *DPOSIllegalBlocks) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := d.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (d *DPOSIllegalBlocks) SerializeUnsigned(w io.Writer, version byte) error {
	if err := common.WriteUint32(w, uint32(d.CoinType)); err != nil {
		return err
	}

	if err := common.WriteUint32(w, d.BlockHeight); err != nil {
		return err
	}

	if err := d.Evidence.SerializeUnsigned(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.SerializeUnsigned(w); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalBlocks) Serialize(w io.Writer, version byte) error {
	if err := d.SerializeUnsigned(w, version); err != nil {
		return err
	}

	if err := d.Evidence.SerializeOthers(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.SerializeOthers(w); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalBlocks) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	var coinType uint32
	if coinType, err = common.ReadUint32(r); err != nil {
		return err
	}
	d.CoinType = CoinType(coinType)

	if d.BlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	if err = d.Evidence.DeserializeUnsigned(r); err != nil {
		return err
	}

	if err = d.CompareEvidence.DeserializeUnsigned(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalBlocks) Deserialize(r io.Reader, version byte) error {
	if err := d.DeserializeUnsigned(r, version); err != nil {
		return err
	}

	if err := d.Evidence.DeserializeOthers(r); err != nil {
		return err
	}

	if err := d.CompareEvidence.DeserializeOthers(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalBlocks) Hash() common.Uint256 {
	if d.hash == nil {
		buf := new(bytes.Buffer)
		d.SerializeUnsigned(buf, IllegalBlockVersion)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		d.hash = &hash
	}
	return *d.hash
}

func (d *DPOSIllegalBlocks) GetBlockHeight() uint32 {
	return d.BlockHeight
}

func (d *DPOSIllegalBlocks) Type() IllegalDataType {
	return IllegalBlock
}
